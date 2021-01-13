const express = require('express');
const path = require('path');

const yts = require('./yts');
const ipc_th = require('./ipc');

const server = express();

server.get('/', (req, res) =>
	res.sendFile(path.join(__dirname, '/index.html'))
);

server.get('/auth', (req, res) => {
	yts.getcode(res);
});

server.get('/callback', (req, res) => {
	const { code } = req.query;
	yts.get_tokens_with_code(code);
	res.redirect('/');
});

server.get('/find-chat', (req, res) => {
	yts.find_chat();
	res.redirect('/');
});

server.get('/start-tracking', (req, res) => {
	yts.start_tracking();
	res.redirect('/');
});

server.get('/stop-tracking', (req, res) => {
	yts.stop_tracking();
	res.redirect('/');
});

server.get('/connect-to-game', (req, res) => {
	ipc_th.connect_to_game();
	res.redirect('/');
});

server.get('/send-message', (req, res) => {
	yts.send_message();
	res.redirect('/');
});


server.listen(3000, () => console.log('Server is running'));
