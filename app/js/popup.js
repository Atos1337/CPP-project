document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('checkCerty').addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndCheck"}, (response) => {});
          }
        );
  document.getElementById('signCerty').addEventListener(
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