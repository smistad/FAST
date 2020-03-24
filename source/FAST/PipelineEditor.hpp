#ifndef FAST_PIPELINE_EDITOR_HPP_
#define FAST_PIPELINE_EDITOR_HPP_

#include <QWidget>
#include <string>
#include "FASTExport.hpp"
#include <QSyntaxHighlighter>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class QTextEdit;

namespace fast {

class PipelineHighlighter;

class FAST_EXPORT PipelineEditor : public QWidget {
    Q_OBJECT
    public:
        PipelineEditor(std::string filename);
        void save();
    private:
        std::string mFilename;
        QTextEdit* mEditor;
        std::string m_memory;
        PipelineHighlighter* highlighter;
    signals:
        /**
         * A signal which is emitted when pipeline file was saved.
         */
        void saved();
};


class PipelineHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    PipelineHighlighter(QTextDocument* parent = 0);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
};

}

#endif
