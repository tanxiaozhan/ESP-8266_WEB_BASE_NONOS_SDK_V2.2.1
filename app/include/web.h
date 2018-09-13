#define INDEX_PAGE "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n\
<html>\
<head>\
<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\
<title>控制卡</title>\
<script>\
function refresh(n){\
	if(n==1)\
		window.TableTXT.rows[0].cells[0].innerHTML=window.dt1.value;\
	else\
		window.TableTXT.rows[1].cells[0].innerHTML=window.dt2.value;\
}\
</script>\
</head>\
<body>\
<table width='90%' border='1' align='center' id='TableTXT'>\
  <tr>\
    <td height='83' align='center' valign='middle'>&nbsp;</td>\
  </tr>\
  <tr>\
    <td height='86' align='center' valign='middle'>&nbsp;</td>\
  </tr>\
</table>\
<form id='form1' name='form1' method='post' action=''>\
    <label for='dt'></label>\
    区域1\
    <select name='dt1' size='1' id='dt1' onchange='refresh(1)'>\
<option>文字1</option>\
      <option>文字2</option>\
      <option>文字3</option>\
      <option>文字4</option>\
      <option>文字5</option>\
    </select>\
    <label for='radio'></label>\
  <p>区域二\
    <select name='dt2' size='1' id='dt2' onchange='refresh(2)'>\
<option>文字1</option>\
      <option>文字2</option>\
      <option>文字3</option>\
      <option>文字4</option>\
      <option>文字5</option> \
	  </select>\
  </p>\
    <label for='fs'></label>\
  字体 \
  <label for='fn'></label>\
  <select name='fn' id='fn'>\
    <option>宋体</option>\
    <option>仿宋</option>\
    <option>楷体</option>\
    <option>黑体</option>\
  </select>\
  <p>字号�\
  <select name='fs' id='fs'>\
    <option>大</option>\
    <option>中</option>\
    <option>小</option>\
  </select>\
  </p>\
    <label for='sp'></label>\
    速度\
    <select name='sp' id='sp'>\
      <option>快速</option>\
      <option>中速</option>\
      <option>慢速</option>\
    </select>\
    <label for='kp'>ͣ停留 </label>\
    <select name='kp' id='kp'>\
      <option>0.2秒</option>\
      <option>0.4秒</option>\
      <option>0.6秒</option>\
    </select>\
    <label for='in'></label>\
    进入 \
    <select name='in' id='in'>\
      <option>从左到右</option>\
      <option>从右到左</option>\
	  </select>\
  <p>线框\
<label for='br'></label>\
    <select name='br' id='br'>\
      <option>无</option>\
      <option>**</option>\
      <option>--</option>\
    </select>\
  </p>\
  <p>\
    <input type='submit' name='button' id='button' value='发送' />\
  </p>\
</form>\
<p>&nbsp;</p>\
</body>\
</html>\
"

#define LOCK_PAGE "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n\
<html>\
<head></head>\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
<body>\
<h1>ESP WiFi NAT Router Config</h1>\
<div id='config'>\
<script>\
if (window.location.search.substr(1) != '')\
{\
document.getElementById('config').display = 'none';\
document.body.innerHTML ='<h1>ESP WiFi NAT Router Config</h1>Unlock request has been sent to the device...';\
setTimeout(\"location.href = '/'\",1000);\
}\
</script>\
<h2>Config Locked</h2>\
<form autocomplete='off' action='' method='GET'>\
<table>\
<tr>\
<td>Password:</td>\
<td><input type='password' name='unlock_password'/></td>\
</tr>\
<tr>\
<td></td>\
<td><input type='submit' value='Unlock'/></td>\
</tr>\
\
</table>\
<small>\
<i>Default: STA password to unlock<br />\
</small>\
</form>\
</div>\
</body>\
</html>\
"
