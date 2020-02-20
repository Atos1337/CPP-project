let ids2filename = {};

chrome.downloads.onChanged.addListener(function(delta) {
  console.log(delta);
  if (!delta.state ||
      (delta.state.current != 'complete')) {
    ids2filename[delta.id] = delta.filename.current;
    return;
  }
  alert(ids2filename[delta.id]);
});
