// Fixed parts of the settings web page (the rest is built in net_portal.ino)
#pragma once

static const char PAGE_HEAD[] = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>YouTube Counter setup</title>
<style>
:root{color-scheme:dark}
body{margin:0;background:#0b0b0d;color:#eee;font:16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
main{max-width:560px;margin:0 auto;padding:16px}
h1{font-size:22px;margin:6px 0 2px;display:flex;align-items:center;gap:10px}
.logo{width:34px;height:24px;background:#f00;border-radius:7px;position:relative;flex:none}
.logo:after{content:"";position:absolute;left:13px;top:7px;border-left:10px solid #fff;border-top:5px solid transparent;border-bottom:5px solid transparent}
.sub{color:#999;font-size:14px;margin:0 0 16px}
fieldset{border:1px solid #2a2a30;border-radius:12px;margin:0 0 14px;padding:12px 14px 4px;background:#141418}
legend{padding:0 6px;color:#ff6b6b;font-weight:600;font-size:13px;letter-spacing:.06em;text-transform:uppercase}
label{display:block;margin:0 0 12px;font-size:14px;color:#bbb}
input:not([type=checkbox]):not([type=range]),select{display:block;width:100%;box-sizing:border-box;margin-top:4px;padding:10px 12px;font-size:16px;color:#fff;background:#0b0b0d;border:1px solid #34343a;border-radius:9px}
input[type=range]{width:100%;margin-top:8px}
.check{display:flex;gap:10px;align-items:flex-start;margin:0 0 12px;color:#ddd}
.check input{margin-top:3px;width:18px;height:18px;flex:none;accent-color:#e00}
.check small,.hint{display:block;color:#888;font-size:12.5px;margin-top:2px}
.row{display:flex;gap:10px}.row>label{flex:1;min-width:0}
button{width:100%;padding:14px;font-size:17px;font-weight:600;color:#fff;background:#e00;border:0;border-radius:12px;margin:4px 0 10px}
button.alt{background:#26262c;font-size:15px;padding:11px}
.msg{padding:12px 14px;border-radius:10px;margin:0 0 14px;background:#2a1a1a;color:#ffb3b3}
.ok{color:#2ecc71}.note{color:#999;font-size:13px}
a{color:#7fb8ff}
</style></head><body><main>
)HTML";

static const char PAGE_SCRIPT[] = R"HTML(<script>
function v(n){var e=document.querySelector('[name='+n+']');return e?e:{checked:false,value:''}}
function est(){
 var r=+v('refresh').value||60,u=Math.round(86400/r)+576;
 if(v('p5').checked)u+=144;
 if(v('p4').checked)u+=101;
 if(v('p7').checked&&v('compare').value.trim())u+=288;
 document.getElementById('quota').textContent='This counter will use about '+u.toLocaleString()+
  ' of the 10,000 free YouTube units a day. Two counters sharing one key should stay under 10,000 together.';
 var b=v('bright');document.getElementById('bv').textContent=Math.round(b.value/2.55)+'%';
}
document.addEventListener('input',est);document.addEventListener('change',est);est();
</script>
)HTML";

static const char PAGE_TAIL[] = "</main></body></html>";
