import QtQuick
import QtQuick.Controls

// Native, passive reflections: no bitmap texture, shader dependency or input layer.
Rectangle {
 id:surface
 readonly property var themeWindow:ApplicationWindow.window
 property bool glossy:!themeWindow||themeWindow.darkTheme
 property real reflection:0.16
 color:"#0A0A0A"
 radius:12
 clip:true
 gradient:glossy?blackGlass:null
 Gradient {
  id:blackGlass
  GradientStop {position:0;color:Qt.lighter(surface.color,1.9)}
  GradientStop {position:0.22;color:surface.color}
  GradientStop {position:0.72;color:Qt.darker(surface.color,1.25)}
  GradientStop {position:1;color:Qt.lighter(surface.color,1.15)}
 }
 Rectangle {
  visible:surface.glossy
  anchors.fill:parent;radius:surface.radius
  gradient:Gradient {
   GradientStop {position:0;color:Qt.rgba(1,1,1,surface.reflection)}
   GradientStop {position:0.11;color:Qt.rgba(1,1,1,surface.reflection*0.28)}
   GradientStop {position:0.42;color:"transparent"}
   GradientStop {position:1;color:"transparent"}
  }
 }
 Rectangle {
  visible:surface.glossy
  x:-surface.width*0.15;y:-height*0.62
  width:surface.width*1.35;height:Math.max(70,surface.height*0.75)
  rotation:-13
  gradient:Gradient {
   GradientStop {position:0;color:Qt.rgba(0.94,0.96,1,surface.reflection*0.75)}
   GradientStop {position:0.48;color:Qt.rgba(1,1,1,surface.reflection*0.22)}
   GradientStop {position:1;color:"transparent"}
  }
 }
 Rectangle {
  visible:surface.glossy
  x:surface.radius;y:1;width:Math.max(0,surface.width-2*surface.radius);height:1
  gradient:Gradient {
   orientation:Gradient.Horizontal
   GradientStop {position:0;color:Qt.rgba(1,1,1,0.02)}
   GradientStop {position:0.15;color:Qt.rgba(0.92,0.95,1,0.4)}
   GradientStop {position:0.58;color:Qt.rgba(1,1,1,0.08)}
   GradientStop {position:1;color:"transparent"}
  }
 }
}
