var port = chrome.runtime.connectNative('com.project.native_messaging_host');

port.onMessage.addListener(function onNativeMessage(msg) {
    var icon;
    if (msg["Create sign"] == "OK") {
        icon = "info"
    }
    else if (msg["Verified"] == "OK") {
        icon = "success";
    }
    else {
        icon = "error";
    }
    var w = 500;
    var h = 500;
    var left = (screen.width/2)-(w/2);
    var top = (screen.height/2)-(h/2); 
    chrome.windows.create({
      url: chrome.runtime.getURL("alert.html"),
      type: "popup",
      width: w,
      height: h,
      left: left,
      top: top
    }, (window) => {
      setTimeout(() => {  chrome.runtime.sendMessage({extensionId: window.id, text: JSON.stringify(msg, null, 2), icon: icon, id: window.id}, (response) => {}); }, 200);
    });
});

port.onDisconnect.addListener(function() {
    console.log("Disconnected");
});

let ids2filename = {};

chrome.downloads.onChanged.addListener(function(delta) {
  if (!delta.state ||
      (delta.state.current != 'complete')) {
    if (delta.filename) {
        ids2filename[delta.id] = delta.filename.current;
    }
    return;
  }
  if (delta.state.current == 'complete') {
    port.postMessage({"request" : "Check certificate in file", "filepath" : ids2filename[delta.id]});
  }
});

function openAndCheckCertificate() {
    port.postMessage({"request" : "Open and check certificate"});
}

function openAndSignCertificate() {
    chrome.storage.sync.get({
        privateKey: "",
        certificate: "",
    }, function(items) {
        port.postMessage({"request" : "Open and sign certificate", "privateKey" : items.privateKey, "certificate" : items.certificate});
    });
}

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
    if (request.message === "Close me pls") {
      sendResponse({message: "OK"});
      chrome.windows.remove(request.id);
    }
    if (request.message === "openAndCheck") {
      sendResponse({message: "OK"});
      openAndCheckCertificate();
    }
    else if (request.message === "openAndSign") {
        sendResponse({message: "OK"});
        openAndSignCertificate();
    }
});