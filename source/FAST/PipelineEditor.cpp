#include "PipelineEditor.hpp"
#include "Exception.hpp"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFontDatabase>
#include <QPushButton>
#include <fstream>
#include <functional>
#include <QDesktopWidget>
#include <QApplication>
#include <QShortcut>

namespace fast {

PipelineEditor::PipelineEditor(std::string filename) {
    mFilename = filename;
    setWindowTitle("FAST - Pipeline Editor");

    std::ifstream file(filename);
    if(!file.is_open())
        throw FileNotFoundException(filename);

    std::string text((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    file.close();

    QVBoxLayout* layout = new QVBoxLayout(this);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mEditor = new QTextEdit();
    mEditor->insertPlainText(text.c_str());
    mEditor->setFont(fixedFont);
    layout->addWidget(mEditor);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* saveButton = new QPushButton();
    saveButton->setText("Save (Ctrl+S)");
    saveButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    QObject::connect(saveButton, &QPushButton::clicked, std::bind(&PipelineEditor::save, this));
    buttonLayout->addWidget(saveButton);

    QPushButton* closeButton = new QPushButton();
    closeButton->setText("Close (Ctrl+Q)");
    closeButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    QObject::connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    // Resize window and put in center
    int width = 700, height = 700;
    QDesktopWidget *desktop = QApplication::desktop();
    int screenWidth = desktop->width();
    int screenHeight = desktop->height();
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;
    move(x, y);
    resize(width, height);

    setLayout(layout);

    // Shortcuts
    QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    QObject::connect(saveShortcut, &QShortcut::activated, std::bind(&PipelineEditor::save, this));
    QShortcut* closeShortcut = new QShortcut(QKeySequence("Ctrl+Q"), this);
    QObject::connect(closeShortcut, &QShortcut::activated, this, &QWidget::close);
}

void PipelineEditor::save() {
    std::ofstream file(mFilename);
    std::string text = std::string(mEditor->toPlainText().toUtf8());
    file << text;
    file.close();
    emit saved();
}


}