const ref = require("ref-napi");
const ffi = require("ffi-napi");

var funcs = ffi.Library('kernel32', {
	'CreateFileA': [ 'int', [ 'string', 'int', 'int', 'int', 'int', 'int', 'int' ] ],
	'WriteFile': [ 'int', [ 'int', 'string', 'int', 'int', 'int' ] ]
});

ipc = {};

let hMail;
var byteRet = ref.refType('int');

ipc.connect_to_game = () => {
	hMail = funcs.CreateFileA("\\\\.\\mailslot\\ThMail", 1073741824, 1, 0, 3, 128, 0);
	if(hMail == -1) {
		console.log("Failed to connect to game");
	}
}

ipc.send_to_game = (msg) => {
	if(typeof(msg) == 'string') {
		funcs.WriteFile(hMail, msg, msg.length + 1, byteRet, 0);
	}
	console.log(msg);
}

module.exports = ipc;