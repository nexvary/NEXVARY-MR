import QtQuick
Canvas {
 id:icon
 property int kind:0
 property color stroke:"#91bfff"
 implicitWidth:28;implicitHeight:28
 onStrokeChanged:requestPaint()
 onPaint:{
  let c=getContext("2d");c.reset();c.strokeStyle=stroke;c.lineWidth=1.7;c.lineCap="round";c.lineJoin="round";
  function line(a,b,d,e){c.beginPath();c.moveTo(a,b);c.lineTo(d,e);c.stroke()}
  function rect(x,y,w,h){c.strokeRect(x,y,w,h)}
  function circle(x,y,r){c.beginPath();c.arc(x,y,r,0,Math.PI*2);c.stroke()}
  if(kind===0){c.beginPath();c.moveTo(3,13);c.lineTo(14,4);c.lineTo(25,13);c.stroke();rect(6,12,16,12);rect(11,17,6,7)}
  else if(kind===1){rect(7,3,14,22);line(11,6,17,6);circle(14,22,0.6)}
  else if(kind===2){circle(12,11,7);line(17,17,24,24);line(6,11,9,11);line(9,11,11,7);line(11,7,13,15);line(13,15,15,11);line(15,11,18,11)}
  else if(kind===3){rect(7,7,14,14);for(let i=0;i<3;i++){let n=9+i*5;line(n,3,n,7);line(n,21,n,25);line(3,n,7,n);line(21,n,25,n)}rect(11,11,6,6)}
  else if(kind===4){line(8,22,8,5);line(4,9,8,5);line(12,9,8,5);line(20,6,20,23);line(16,19,20,23);line(24,19,20,23);line(14,13,14,15)}
  else if(kind===5){rect(6,4,17,21);rect(10,2,9,5);line(10,12,19,12);line(10,16,19,16);line(10,20,16,20)}
  else if(kind===6){rect(4,8,20,16);line(4,13,24,13);line(14,8,14,24);line(4,8,14,3);line(14,3,24,8)}
  else{circle(14,14,5);for(let i=0;i<8;i++){let a=i*Math.PI/4;line(14+Math.cos(a)*9,14+Math.sin(a)*9,14+Math.cos(a)*12,14+Math.sin(a)*12)}circle(14,14,9)}
 }
}
