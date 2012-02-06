import QtQuick 1.1

Rectangle {
  property string title: ""
  property real titleHeight: title !== "" ? 15 : 0
  property real padding: 5
  property variant subMenuItem: null

  id: contents
  width: 100
  height: list.height + padding*2.0 + titleHeight
  color: "#222222"
  border.width: 1
  border.color: "#555555"
  radius: 3
  smooth: true
  property variant menuItems: null

  onMenuItemsChanged:
    {
    contentsModel.clear();
    for (var prop in menuItems)
      {
      var data = menuItems[prop];
      contentsModel.append({"name": prop, "description":data.description, hasSubMenu: data.children !== undefined })
      }
    }

  function expand()
    {
    var tl = contents.mapToItem(null, 0, 0);

    var pos = { x: 0.0, y: 0.0 };
    if(0.0 > tl.x)
      {
      pos.x = tl.x;
      }
    if(0.0 > tl.y)
      {
      pos.y = tl.y;
      }
    holder.move(pos);


    var br = contents.mapToItem(null, contents.width, list.childrenRect.height + padding*2 + titleHeight);
    br.x -= pos.x;
    br.y -= pos.y;

    if((holder.x + holder.width) < br.x)
      {
      holder.width = Math.max(holder.width, br.x); // + holder.x;
      }
    if((holder.y + holder.height) < br.y)
      {
      holder.height = Math.max(holder.height, br.y); // + holder.y;
      }
    }

  state: "Hidden"
  states: [
    State {
      name: "Hidden"

      PropertyChanges {
        target: contents
        opacity: 0.0
        }
      PropertyChanges {
        target: list
        height: 0.0
        }
      },

    State {
      name: "Shown"

      PropertyChanges {
        target: contents
        opacity: 1.0
        }
      PropertyChanges {
        target: list
        height: list.childrenRect.height
        }
      },

    State {
      name: "Done"

      PropertyChanges {
        target: contents
        opacity: 0.0
        }
      }
    ]

  transitions: [
    Transition {
      from: "*"; to: "Done"
      SequentialAnimation {
        ParallelAnimation {
          PropertyAnimation { property: "opacity"; easing.type: Easing.OutCubic; duration: 70 }
          PropertyAnimation { property: "height"; easing.type: Easing.OutBounce; duration: 150 }
          }

        ScriptAction {
          script: {
            contents.destroy();
            }
          }
        }
      },
    Transition {
      from: "*"; to: "*"
      PropertyAnimation { property: "opacity"; easing.type: Easing.OutCubic; duration: 70 }
      PropertyAnimation { property: "height"; easing.type: Easing.OutBounce; duration: 150 }
      }
    ]

  Text {
    text: title
    color: "white"
    font.bold: true
    x: 5
    y: (padding + titleHeight)/2.0 - height/2.0
  }

  Component.onCompleted: {
    external.focusLost.connect(focusLost);
    grabber.start();
  }

  Column {
    id: list
    y: padding + titleHeight

    width: parent.width
    height: childrenRect.height

    Repeater {
      model: ListModel {
        id: contentsModel
        }

      delegate: Rectangle {
        id: item
        border.width: 1
        border.color: "#333333"
        width: parent ? parent.width : 0
        height: column.height

        states: [
          State {
            when: mouse.pressed
            PropertyChanges {
              target: item
              color: "#373737"
            }
          },
          State {
            when: !mouse.containsMouse
            PropertyChanges {
              target: item
              color: "#444444"
            }
          },
          State {
            when: mouse.containsMouse
            PropertyChanges {
              target: item
              color: "#555555"
            }
          }
        ]

        function triggerSubMenu() {
          if(subMenuItem)
            {
            subMenuItem.state = "Done";
            subMenuItem = null;
            }

          var component = Qt.createComponent("Menu.qml");
          var object = component.createObject(item);

          var i = 0;
          var data = null;
          for (var prop in menuItems)
            {
            if(i === index)
              {
              data = menuItems[prop].children;
              }
            ++i;
            }

          object.menuItems = data;
          object.title = "";
          object.x = (function(){ return item.x + item.width + 1; });
          object.y = -padding;
          object.z = 2;
          object.state = "Shown";
          object.expand();


          subMenuItem = object;
        }

        Timer {
          id: subMenu
          interval: 250
          onTriggered: triggerSubMenu()
        }

        MouseArea
          {
          id: mouse
          anchors.fill: parent
          hoverEnabled: true
          onEntered: {
            if(hasSubMenu)
              {
              subMenu.start();
              }
            }
          onExited: subMenu.stop();
          onClicked: {
            if(hasSubMenu)
              {
              triggerSubMenu()
              return;
              }

            var request = null;
            var data = null;
            var i = 0;
            for (var prop in menuItems)
              {
              if(i === index)
                {
                request = menuItems[prop].request;
                data = menuItems[prop].requestData;
                }
              ++i;
              }

            external.emitRequest(request, data);
            external.destroyWindow();
            }
          }

        Column {
          id: column
          width: item.width
          height: childrenRect.height

          Text {
            x: 2
            text: name
            font.bold: true
            color: "white"
            }
          Text {
            x: 2
            text: description
            font.pixelSize: 9
            color: "white"
            }
          }

        Text {
          visible: hasSubMenu
          text: ">"
          font.bold: true
          color: "white"

          anchors.margins: 2.0
          anchors.verticalCenter: column.verticalCenter
          anchors.right: column.right
          }
        }
      }
    }
  }
