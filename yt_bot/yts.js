const { google } = require('googleapis');
const fs = require('fs');
const yt = google.youtube('v3');
const ipc_th = require('./ipc');

let liveChatId;
let next_page;
const interval_time = 200;
let interval;
let chat_messages = [];

const oauth2 = google.auth.OAuth2;
const cdata = JSON.parse(fs.readFileSync("client.json"));
const auth = new oauth2(cdata.cid, cdata.csecret, "http://localhost:3000/callback");

const scope = [
	'https://www.googleapis.com/auth/youtube.readonly',
	'https://www.googleapis.com/auth/youtube',
	'https://www.googleapis.com/auth/youtube.force-ssl'
];

const yts = {};

yts.getcode = res => {
	const authurl = auth.generateAuthUrl({
		access_type: 'offline',
		scope
	});
	res.redirect(authurl);
}

yts.authorize = ({ tokens }) => {
	auth.setCredentials(tokens);
	console.log("Successfully set credentials");
	console.log('tokens', tokens);
	fs.writeFileSync('./tokens.json', JSON.stringify(tokens));
}

auth.on('tokens', (tokens) => {
	console.log("Tokens refreshed");
	fs.writeFileSync('./tokens.json', JSON.stringify(tokens));
});

async function check_tokens() {
	const tokens = await JSON.parse(fs.readFileSync("./tokens.json"));
	if(tokens) {
		console.log("Tokens read from file");
		return auth.setCredentials(tokens);
	}
	console.log("No tokens found");
}

yts.find_chat = async () => {
	const response = await yt.liveBroadcasts.list({
		auth,
		part: 'snippet',
		broadcastStatus: 'active'
	});
	liveChatId = response.data.items[0].snippet.liveChatId;
	console.log("Chat ID: ", liveChatId);
}

async function get_chat_messages() {
	const response = await yt.liveChatMessages.list({
		auth,
		part: 'snippet',
		liveChatId,
		pageToken: next_page
	});
	const {data} = response;
	const new_messages = data.items;
	chat_messages.push(...new_messages);
	next_page = data.nextPageToken;
	
	new_messages.forEach(e => {
		ipc_th.send_to_game(e.snippet.authorChannelId + e.snippet.textMessageDetails.messageText);
	});
	
	// console.log("Total messages: ", chat_messages.length);
}

yts.start_tracking = async () => {
	interval = setInterval(get_chat_messages, interval_time);
};

yts.stop_tracking = () => {
	clearInterval(interval);
};

yts.send_message = async () => {
	yt.liveChatMessages.insert({
		auth,
		part: 'snippet',
		resource: {
			snippet: {
				liveChatId,
				type: "textMessageEvent",
				textMessageDetails: {
					messageText: "Test Message"
				}
			}
		}
	});
};

check_tokens();

yts.get_tokens_with_code = async code => {
	const cred = await auth.getToken(code);
	yts.authorize(cred);
}

module.exports = yts;