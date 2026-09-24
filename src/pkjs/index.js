function currentSettings() {
  return { use12h: localStorage.getItem('use12h') === 'true' };
}

function sendSettings(cfg) {
  Pebble.sendAppMessage(
    { 'USE_12H': cfg.use12h ? 1 : 0 },
    function() { console.log('Settings sent: ' + JSON.stringify(cfg)); },
    function(err) { console.log('Failed to send settings: ' + JSON.stringify(err)); }
  );
}

var CONFIG_HTML =
  '<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width">' +
  '<style>body{font-family:-apple-system,sans-serif;margin:20px;background:#f5f5f5;color:#111}' +
  '.card{background:#fff;border-radius:8px;padding:16px;margin-bottom:12px;box-shadow:0 1px 3px rgba(0,0,0,.12)}' +
  '.row{display:flex;justify-content:space-between;align-items:center}' +
  'label{font-size:16px}input[type=checkbox]{width:22px;height:22px}' +
  'button{width:100%;padding:14px;background:#007aff;color:#fff;border:none;border-radius:8px;font-size:16px;margin-top:16px}' +
  '</style></head><body><h2>Dygn</h2>' +
  '<div class="card"><div class="row"><label for="use12h">12-hour numerals</label>' +
  '<input type="checkbox" id="use12h"></div></div>' +
  '<button onclick="submit()">Save</button>' +
  '<script>' +
  'var o=JSON.parse(decodeURIComponent(location.hash.substring(1))||"{}");' +
  'document.getElementById("use12h").checked=!!o.use12h;' +
  'function submit(){var r={use12h:document.getElementById("use12h").checked};' +
  'document.location="pebblejs://close#"+encodeURIComponent(JSON.stringify(r))}' +
  '</script></body></html>';

Pebble.addEventListener('ready', function() {
  sendSettings(currentSettings());
});

Pebble.addEventListener('showConfiguration', function() {
  var hash = encodeURIComponent(JSON.stringify(currentSettings()));
  Pebble.openURL('data:text/html,' + encodeURIComponent(CONFIG_HTML) + '#' + hash);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;
  var cfg = JSON.parse(decodeURIComponent(e.response));
  localStorage.setItem('use12h', cfg.use12h);
  sendSettings(cfg);
});
