function appendMessage(text) {
    document.getElementById('response').innerHTML += "<p>" + text + "</p>";
}

function sendNativeMessage() {
    message = {"text": document.getElementById('input-text').value};
    port.postMessage(message);
}

function onNativeMessage(msg) {
    appendMessage("Received message: <b>" + JSON.stringify(msg) + "</b>");
}

var port = chrome.runtime.connectNative('com.project.native_messaging_host');

port.onMessage.addListener(onNativeMessage);

port.onDisconnect.addListener(function() {
  console.log("Disconnected");
});

document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('send-message-button').addEventListener(
        'click', sendNativeMessage);
}); 
