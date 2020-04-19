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
}); 

//