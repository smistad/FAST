#ifndef FAST_PIPELINE_EDITOR_HPP_
#define FAST_PIPELINE_EDITOR_HPP_

#include <QWidget>
#include <string>
#include "FASTExport.hpp"

class QTextEdit;

namespace fast {

class FAST_EXPORT PipelineEditor : public QWidget {
    Q_OBJECT
    public:
        PipelineEditor(std::string filename);
        void save();
    private:
        std::string mFilename;
        QTextEdit* mEditor;
    signals:
        /**
         * A signal which is emitted when pipeline file was saved.
         */
        void saved();
};

}

#endif
