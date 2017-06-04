#ifndef FAST_PIPELINE_EDITOR_HPP_
#define FAST_PIPELINE_EDITOR_HPP_

#include <QWidget>
#include <string>

class QTextEdit;

namespace fast {

class PipelineEditor : public QWidget {
    public:
        PipelineEditor(std::string filename);
        void save();
    private:
        std::string mFilename;
        QTextEdit* mEditor;
};

}

#endif
