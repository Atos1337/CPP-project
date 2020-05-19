document.addEventListener('DOMContentLoaded', function () {
  checkButton = document.getElementById('checkZip');
  signButton = document.getElementById('signZip');

  chrome.storage.sync.get({
    privateKey: null,
    certificate: null,
    certificatesStore: null
  }, function(items) {  
    if (!items.privateKey || !items.certificate) {
      signButton.setAttribute('disabled', true);
    }
    if (!items.certificatesStore || !items.certificatesStore.length) {
      checkButton.setAttribute('disabled', true);
    }
  });

  checkButton.addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndCheck"}, (response) => {});
          }
        );
  signButton.addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndSign"}, (response) => {});
          }
        );
  document.getElementById('settingsButton').addEventListener('click', function (event) {
    if (chrome.runtime.openOptionsPage) {
      chrome.runtime.openOptionsPage();
    } 
    else {
      window.open(chrome.runtime.getURL('options.html'));
    }
  }, true);
}); 

//