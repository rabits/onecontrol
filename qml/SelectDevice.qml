import QtQuick 2.7
import QtQuick.Controls 2.0
import org.rabits.onecontrol 1.0

Rectangle {
    id: select_device

    signal deviceSelected(string name, string address)

    BusyIndicator {
        id: busy_indicator
        anchors.fill: parent
        opacity: 0.1

        Text {
            anchors.fill: parent
            text: qsTr("Scanning for devices...")

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    ListView {
        id: devices_list

        function deviceFound(name, address) {
            console.log("Found new device: " + name + ' (' + address + ')')
            devices_list.model.append({"name": name, "address": address})
        }

        anchors {
            fill: parent
            margins: 8
        }

        clip: true

        model: ListModel {}

        delegate: Rectangle {
            width: devices_list.width
            height: 40
            color: "#bbeeeeee"
            Column {
                Text {
                    text: name
                    font.bold: true
                }
                Text {
                    text: address
                    color: "grey"
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: deviceSelected(name, address)
            }
        }

        Component.onCompleted: {
            app.bluetooth().deviceFound.connect(devices_list.deviceFound)
            if( visible )
                app.bluetooth().discoveryStart()
        }
    }

    onVisibleChanged: {
        if( visible ) {
            app.bluetooth().discoveryStart()
        } else {
            app.bluetooth().discoveryStop()
            devices_list.model.clear()
        }
    }
}
