var port = chrome.runtime.connectNative('com.project.native_messaging_host');

port.onMessage.addListener(function onNativeMessage(msg) {});

port.onDisconnect.addListener(function() {
  console.log("Disconnected");
});

let ids2filename = {};

chrome.downloads.onChanged.addListener(function(delta) {
  console.log(delta);
  if (!delta.state ||
      (delta.state.current != 'complete')) {
    ids2filename[delta.id] = delta.filename.current;
    return;
  }
  port.postMessage({"file" : ids2filename[delta.id]});
});
