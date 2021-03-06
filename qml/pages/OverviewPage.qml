import QtQuick 2.0
import Sailfish.Silica 1.0

import org.nubecula.harbour.apocalypse 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    PageBusyIndicator {
        id: busyIndicator
        size: BusyIndicatorSize.Large
        running: ServiceProvider.loading
        anchors.centerIn: parent
    }

    SilicaListView {
        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("All Messages")
                onClicked: pageStack.push(Qt.resolvedUrl("MessageListPage.qml"))
            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: ServiceProvider.refresh();
            }

        }

        id: listView

        opacity: ServiceProvider.loading ? 0.4 : 1.0
        Behavior on opacity { FadeAnimator {} }

        model: MessageSortFilterModel {
            id: sortModel
            sourceModel: ServiceProvider.messageModel()

            Component.onCompleted: showLocalOnly()
        }

        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Local Messages")
        }
        delegate: ListItem {
            id: delegate

            width: parent.width
            contentHeight: Theme.itemSizeLarge

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                height: parent.height
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    id: itemIcon
                    source: "image://theme/icon-m-media-radio"

                    anchors.verticalCenter: parent.verticalCenter
                }

                Item {
                    width: Theme.paddingMedium
                    height: 1
                }

                Column {
                    width: parent.width - itemIcon.width - Theme.paddingMedium
                    anchors.verticalCenter: itemIcon.verticalCenter

                    Label {
                        width: parent.width
                        text: event_title.toUpperCase()
                        color: pressed ? Theme.secondaryHighlightColor : Theme.highlightColor
                        font.pixelSize: Theme.fontSizeMedium
                    }
                    Label {
                        text: sender_name

                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeExtraSmall
                    }
                }
            }
            onClicked: pageStack.push(Qt.resolvedUrl("MessagePage.qml"), { msg: ServiceProvider.messageModel().messageAt(idx) })
        }

        ViewPlaceholder {
            enabled: listView.count == 0
            text: qsTr("No local messages")
            hintText: qsTr("There are no local messages available")
        }

        VerticalScrollDecorator {}
    }
}
