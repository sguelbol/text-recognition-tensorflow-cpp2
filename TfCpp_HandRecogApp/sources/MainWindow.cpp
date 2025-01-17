#include <QtWidgets>

#include "../headers/MainWindow.h"
#include "../headers/ScribbleArea.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    scribbleArea = new ScribbleArea;
    createActions();
    createMenus();

    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scribbleArea->setSizePolicy(sizePolicy);
    mainLayout->addWidget(scribbleArea);

    createTrainingField(*mainLayout);

    setCentralWidget(widget);
    setWindowTitle(tr("Scribble"));
}

void MainWindow::createTrainingField(QVBoxLayout &mainLayout) {
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    button = new QPushButton("Retrain");
    button->setStyleSheet("background-color: red;");
    textField = new QLineEdit;
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::white);
    palette.setColor(QPalette::Text,Qt::black);
    textField->setPalette(palette);
    QValidator *validator = new QIntValidator(0, 9, this);
    textField->setValidator(validator);
    bottomLayout->addStretch(1);
    textField->setMaximumWidth(100);
    button->setMaximumWidth(100);
    bottomLayout->addWidget(textField);
    bottomLayout->addWidget(button);
    bottomLayout->addStretch(1);
    mainLayout.addLayout(bottomLayout);
    connect(button, SIGNAL (clicked()), this, SLOT (trainOnWrittenChar()));
}

void MainWindow::trainOnWrittenChar() {
    int expectedNumber = textField->text().toInt();
    textField->clear();
    scribbleArea->trainOnWrittenChar(expectedNumber);
}

void MainWindow::addModel(const shared_ptr<Model> model) {
    scribbleArea->setModel(model);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        event ->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::open() {
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
        if (!fileName.isEmpty()) {
            scribbleArea->openImage(fileName);
        }
    }
}

void MainWindow::save() {
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    saveFile(fileFormat);
}

void MainWindow::penColor() {
    QColor newColor = QColorDialog::getColor(scribbleArea->penColor());
    if (newColor.isValid()) {
        scribbleArea->setPenColor(newColor);
    }
}

void MainWindow::penWidth() {
    bool ok;
    int newWidth = QInputDialog::getInt(this, tr("Scribble"), tr("Select pen width: "), scribbleArea->penWidth(), 1, 50, 1, &ok);
    if (ok) {
        scribbleArea->setPenWidth(newWidth);
    }
}


void MainWindow::about() {
    QMessageBox::about(this, tr("About Scribble"), tr("<p>The <b>Scribble</b> example is awesome</p>"));
}

void MainWindow::createActions() {
    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcut(QKeySequence::Open); // For showing the shortcut
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
            foreach(QByteArray format, QImageWriter::supportedImageFormats()) {
                QString text = tr("%1...").arg(QString(format).toUpper());
                QAction *action = new QAction(text, this);
                action->setData(format);
                connect(action, SIGNAL(triggered()), this, SLOT(save()));
                saveAsActs.append(action);
            }

    printAct = new QAction(tr("&Print..."), this);
    connect(printAct, SIGNAL(triggered()), scribbleArea, SLOT(print()));
    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    penColorAct = new QAction(tr("&Pen Color..."), this);
    connect(penColorAct, SIGNAL(triggered()), this, SLOT(penColor()));
    penWidthAct = new QAction(tr("&Pen Width..."), this);
    connect(penWidthAct, SIGNAL(triggered()), this, SLOT(penWidth()));

    clearScreenAct = new QAction(tr("&Clear Screen..."), this);
    clearScreenAct->setShortcut(tr("Ctrl+L"));
    connect(clearScreenAct, SIGNAL(triggered()), scribbleArea, SLOT(clearImage()));

    aboutAct = new QAction(tr("&About..."), this);
    connect(aboutAct, SIGNAL(triggered()), SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt..."), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

// Create menu bar
void MainWindow::createMenus() {
    saveAsMenu = new QMenu(tr("&Save As"), this);
            foreach(QAction *action, saveAsActs) saveAsMenu->addAction(action);
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    optionMenu = new QMenu(tr("&Options"), this);
    optionMenu->addAction(penColorAct);
    optionMenu->addAction(penWidthAct);
    optionMenu->addSeparator();
    optionMenu->addAction(clearScreenAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(optionMenu);
    menuBar()->addMenu(helpMenu);
}

bool MainWindow::maybeSave() {
    if(scribbleArea->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Scribble"), tr("The image has been modified.\n" "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if(ret == QMessageBox::Save) {
            return saveFile("png");
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}


bool MainWindow::saveFile(const QByteArray &fileFormat) {
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath,
                                                    tr("%1 Files (*.%2);; All Files(*)")
                                                            .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                            .arg(QString::fromLatin1(fileFormat)));
    if (fileName.isEmpty()) {
        return false;
    } else {
        return scribbleArea->saveImage(fileName, fileFormat.constData());
    }
}