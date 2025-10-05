#include <QApplication>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <iostream>
#include <X11/Xlib.h>
#include <csignal>
#include <cstdio>
#include <thread>

#include "AutoClicker.h"

using namespace std;

static AutoClicker autoClicker;


void handle_sigint(int) { autoClicker.shutdown(); }

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Autoclicker");
    auto *input = new QLineEdit(&window);
    input->setPlaceholderText("Enter an integer value");
    auto *button = new QPushButton("Save", &window);
    auto *layout = new QVBoxLayout();

    layout->addWidget(input);
    layout->addWidget(button);
    window.setLayout(layout);

    QObject::connect(button, &QPushButton::clicked, [&]() {
        bool ok;
        int value = input->text().toInt(&ok);
        if (!ok) {
            QMessageBox::warning(&window, "Invalid input", QObject::tr("Invalid input"));
            return;
        }
        autoClicker.setDelay(value);
        cout << "delay set to " << value << endl;
    });

    window.show();

    signal(SIGINT, handle_sigint);
    return app.exec();
}
