import QtQuick 2.7
import QtQuick.Controls 2.0
import org.rabits.onecontrol 1.0
import QtWebView 1.1
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: window
    title: "OneControl"
    width: 360
    height: 640
    visible: true

    header: Rectangle {
        height: window.height / 15
        color: "#000000"
        Text {
            id: device_name
            anchors.fill: parent

            color: "#eeeeee"
            text: qsTr("Device not selected")

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    StackLayout {
        id: swipe
        anchors.fill: parent
        currentIndex: tabbar.currentIndex

        Rectangle {
            id: onebutton_tab
            color: "#aa0000"
            Rectangle {
                id: audio_volume
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                height: parent.height/3
                color: "#bb0000"
            }
            Rectangle {
                id: system_poweroff
                anchors {
                    top: audio_volume.bottom
                    left: parent.left
                    right: parent.right
                }
                height: parent.height/3
                color: "#cc0000"
            }
            Rectangle {
                id: update_onebutton
                anchors {
                    top: system_poweroff.bottom
                    left: parent.left
                    right: parent.right
                }
                height: parent.height/3
                color: "#dd0000"
            }
        }
        Rectangle {
            id: guitarix_tab
            color: "#00aa00"
            WebView {
                id: guitarix_webview
                anchors.fill: parent
                url: ""
            }
        }
    }

    SelectDevice {
        id: select_device
        anchors.fill: parent
        visible: true

        property string name: ""
        property string address: ""
        property bool connected: false

        onDeviceSelected: {
            select_device.name = name
            select_device.address = address
            app.bluetooth().connectTo(address)
            device_name.text = qsTr("Connecting ") + name + " (" + address + ")"
        }

        Connections {
            target: app.bluetooth()
            onStateChanged: {
                device_name.text = (connected ? qsTr("Connected ") : qsTr("Disconnected ")) + select_device.name + " (" + select_device.address + ")"
                select_device.connected = connected
                select_device.visible = ! connected
                if( connected ) {
                    cfg.setting('bluetooth/last_device_name', select_device.name)
                    cfg.setting('bluetooth/last_device_address', select_device.address)
                }
            }
        }
    }

    footer: Item {
        height: window.height / 12

        TabBar {
            id: tabbar
            anchors.left: parent.left
            anchors.right: menu_button.left
            currentIndex: swipe.currentIndex

            width: parent.width
            height: parent.height
            TabButton {
                text: qsTr("OneButton")
                enabled: select_device.connected
                height: parent.height
                onClicked: {
                }
            }
            TabButton {
                text: qsTr("Guitarix")
                enabled: select_device.connected
                height: parent.height
                onClicked: {
                    guitarix_webview.url = "http://localhost:" + cfg.setting('bluetooth/service_guitarix_web_port') + "/"
                }
            }
        }
        Button {
            id: menu_button

            anchors.right: parent.right

            text: "..."
            onClicked: menu.open()
            height: parent.height
            width: height

            Menu {
                id: menu
                x: menu_button.width - width

                MenuItem {
                    text: "Select Device"
                    onClicked: select_device.visible = !select_device.visible
                }
                MenuItem {
                    text: "Settings"
                    onClicked: {
                    }
                }
                MenuItem {
                    text: "About"
                    onClicked: {
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if( cfg.setting('bluetooth/last_device_address') ) {
            select_device.deviceSelected(cfg.setting('bluetooth/last_device_name'),
                                         cfg.setting('bluetooth/last_device_address'))
        }
    }
}
