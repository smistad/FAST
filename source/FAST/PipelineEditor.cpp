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
    highlighter = new PipelineHighlighter(mEditor->document());
    mEditor->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #dadada; }");
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
}

void PipelineEditor::save() {
    std::ofstream file(mFilename);
    QString s = mEditor->toPlainText().toUtf8();
    std::string text = s.toStdString();
    file << text;
    file.close();
    emit saved();
}

PipelineHighlighter::PipelineHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    keywordFormat.setForeground(QColor("#d8a0df"));
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bProcessObject\\b"),
        QStringLiteral("\\bRenderer\\b"),
        QStringLiteral("\\bInput\\b"),
        QStringLiteral("\\bAttribute\\b"),
        QStringLiteral("\\bView\\b"),
        QStringLiteral("\\bPipelineName\\b"),
        QStringLiteral("\\bPipelineDescription\\b"),
    };
    for(const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    quotationFormat.setForeground(QColor("#d69d85"));
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Testasd
    singleLineCommentFormat.setForeground(QColor("#57a64a"));
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void PipelineHighlighter::highlightBlock(const QString& text) {
    for(const HighlightingRule& rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while(matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

}