var port = chrome.runtime.connectNative('com.project.native_messaging_host');
function isTrusted(certificate, callback) {
  if (!certificate) {
    callback(false);
    return;
  }
  chrome.storage.sync.get({
   certificatesStore: null
  }, function(items) { 
    find = false; 
    if (items.certificatesStore) {
      for (let i = 0; i < items.certificatesStore.length; i++) {
        if (items.certificatesStore[i].trim() == certificate.trim()) {
          find = true;
          break;
        }
      }
    }
    callback(find);
  });
}

function prettyNativeMessageFormater(msg, callback) {
  function prettyCertificateDataFormater(data) {
    function processValue(name, value) {
      return "<div>" + name + " " + value + "</div>";
    }
    return processValue("Страна", data["C"]) + 
              processValue("Город", data["ST"]) +
              processValue("Населённый пункт", data["L"]) +
              processValue("Организация", data["O"]) +
              processValue("Подразделение организации", data["OU"]) +
              processValue("Имя", data["CN"]) +
              processValue("Почта", data["emailAddress"]);
  }
  isTrusted(msg["Certificate"], (trusted) => {
    var icon, text;
    if (msg["Create sign"] == "OK") {
      icon = "info";
      text = "Файл " + msg["ArchiveName"] + " успешно подписан";
    }
    else if (msg["Verified"] == "OK") {
      if (trusted) {
        icon = "success";
        text = "Подпись архива " + msg["ArchiveName"] + " совпала, Вы доверяете этому сертификату";
      }
      else {
        icon = "warning";
        text = "Подпись архива " + msg["ArchiveName"] + " совпала, но данного сертификата нет в Вашем списке доверенных"
      }
      text += '<div class="certificateData">' + prettyCertificateDataFormater(msg["CertificateData"]) + "</div>";
    }
    else {
      icon = "error";
      text = "Подпись архива " + msg["ArchiveName"] + " не совпала или не найдена";
    }
    callback(icon, text);
  });
}

port.onMessage.addListener(function onNativeMessage(msg) {
    prettyNativeMessageFormater(msg, (icon, text) => {
      var w = 500;
      var h = 500;
      var left = (screen.width / 2) - (w / 2);
      var top = (screen.height / 2) - (h / 2); 
      chrome.windows.create({
        url: chrome.runtime.getURL("alert.html"),
        type: "popup",
        width: w,
        height: h,
        left: left,
        top: top
      }, (window) => {
        setTimeout(() => {  
          chrome.runtime.sendMessage({extensionId: window.id, text: text, icon: icon, id: window.id}, (response) => {}); 
        }, 500);
      });
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
    var re = /(?:\.([^.]+))?$/;
    if (re.exec(ids2filename[delta.id])[1] == "zip") {
      port.postMessage({"request" : "Check certificate in file", "filepath" : ids2filename[delta.id]});
    }
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