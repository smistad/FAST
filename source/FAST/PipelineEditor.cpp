#include "PipelineEditor.hpp"
#include "Exception.hpp"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFontDatabase>
#include <QPushButton>
#include <fstream>
#include <functional>
#include <QScreen>
#include <QApplication>
#include <QShortcut>
#include <FAST/Utility.hpp>
#include <QTimer>

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

    const QFont fixedFont("UbuntuMono");
    mEditor = new QTextEdit();
    mEditor->setStyleSheet("QTextEdit { background-color: #1f1f1f; color: #dddddd; }");
    mEditor->insertHtml(replace(text, "\n", "<br>").c_str());
    mEditor->setFont(fixedFont);
    auto timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    //connect(timer, &QTimer::timeout, this, &PipelineEditor::syntaxHighlightUpdate);
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
    QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
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
    //timer->start();
}

void PipelineEditor::syntaxHighlightUpdate() {
    if(m_memory == mEditor->toHtml().toStdString()) // Nothing has changed
        return;
    QString s = mEditor->toPlainText().toUtf8();
    s = s.replace("<br>", "\n");
    s = s.remove(QRegExp("<[^>]*>"));
    const std::string text = s.toStdString();
    std::string newText = "";
    auto lines = split(text, "\n", false);
    std::cout << "Found " << lines.size() << " lines" << std::endl;
    for(auto line : lines) {
        std::cout << line << std::endl;
        auto pos = line.find(' ');
        if(pos != std::string::npos) {
            auto firstWord = line.substr(0, pos);
            std::cout << firstWord << std::endl;
            if(firstWord == "ProcessObject" || firstWord == "Renderer") {
                newText += "<b>" + firstWord + "</b> " + line.substr(pos + 1) + "<br>";
            } else {
                newText += line + "<br>";
            }
        } else {
            newText += line + "<br>";
        }
    }
    std::cout << newText << std::endl;
    mEditor->setHtml(newText.c_str());
    m_memory = mEditor->toHtml().toStdString();
}

void PipelineEditor::save() {
    std::ofstream file(mFilename);
    QString s = mEditor->toPlainText().toUtf8();
    s = s.replace("<br>", "\n");
    s = s.remove(QRegExp("<[^>]*>"));
    std::string text = s.toStdString();
    file << text;
    file.close();
    emit saved();
}


}