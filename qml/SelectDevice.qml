import QtQuick 2.6

Rectangle {
    id: select_device

    height: 1280
    width: 720

    color: "#ffffff"

    Rectangle {
        id: header
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            left: parent.left
            margins: 8
            bottomMargin: parent.height * 0.8
        }

        color: "#7de27d"

        Text {
            id: header_caption
            anchors.fill: parent

            text: qsTr("OneControl")
            visible: true

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            font {
                bold: true
                pointSize: 25
                family: "Verdana"
            }
        }
    }

    ListView {
        id: devices_list
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 8
        }

        clip: true

        model: ListModel {
            ListElement {
                name: "Grey"
                colorCode: "grey"
            }

            ListElement {
                name: "Red"
                colorCode: "red"
            }

            ListElement {
                name: "Blue"
                colorCode: "blue"
            }

            ListElement {
                name: "Green"
                colorCode: "green"
            }
        }
        delegate: Item {
            x: 5
            width: 80
            height: 40
            Row {
                id: row1
                spacing: 10
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                }

                Text {
                    text: name
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
