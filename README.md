# thchat
Hooks up a YouTube chat to a Touholu game, allowing chat messages to affect in-game enemy behaviour

## yt_bot
The Node.js code for the YouTube bot that recieves the chat messages

## thcrap_plugin
A C++ DLL that integrates into thcrap's plugin system to recieve data from yt_bot.

## thcrap_patch
A thcrap patch that uses the thcrap plugin and specifies game specific information (memory addresses, struct offsets)
