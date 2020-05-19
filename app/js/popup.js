function disableWithHint(button, hint) {
  button.setAttribute('disabled', true);
  button.text = hint;
}

document.addEventListener('DOMContentLoaded', function () {
  checkButton = document.getElementById('checkCerty');
  signButton = document.getElementById('signCerty');

  //disableWithHint(checkButton, "Выберите доверенные сертификаты в настройках");
  //disableWithHint(signButton, "Выберите ключи в настройках");

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