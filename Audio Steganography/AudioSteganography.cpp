#include "AudioSteganography.h"
#include "AudioSteganographyCode.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include "audioVisualizer.h"
#include <QAudioOutput>



AudioSteganography::AudioSteganography(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    inputPlayer = new QMediaPlayer(this);
    outputPlayer = new QMediaPlayer(this);

    QAudioOutput* audioOutput = new QAudioOutput;
    inputPlayer->setAudioOutput(audioOutput);

    QAudioOutput* audioOutput2 = new QAudioOutput;
    outputPlayer->setAudioOutput(audioOutput2);

    inputAudioSlider = ui.inputAudioSlider;
    outputAudioSlider = ui.outputAudioSlider;

    connect(inputAudioSlider, &QSlider::sliderMoved, inputPlayer, &QMediaPlayer::setPosition);
    connect(outputAudioSlider, &QSlider::sliderMoved, outputPlayer, &QMediaPlayer::setPosition);
    connect(ui.playInputButton, &QPushButton::clicked, this, &AudioSteganography::onPlayInputButtonClicked);
    connect(ui.playOutputButton, &QPushButton::clicked, this, &AudioSteganography::onPlayOutputButtonClicked);\
    connect(inputPlayer, &QMediaPlayer::positionChanged, this, [&](qint64 pos) {
        inputAudioSlider->blockSignals(true);
        inputAudioSlider->setValue(pos);
        inputAudioSlider->blockSignals(false);
        });
    connect(inputPlayer, &QMediaPlayer::durationChanged, this, [&](qint64 dur) {
        inputAudioSlider->setMaximum(dur);
        });

    connect(outputPlayer, &QMediaPlayer::positionChanged, this, [&](qint64 pos) {
        outputAudioSlider->blockSignals(true);
        outputAudioSlider->setValue(pos);
        outputAudioSlider->blockSignals(false);
        });
    connect(outputPlayer, &QMediaPlayer::durationChanged, this, [&](qint64 dur) {
        outputAudioSlider->setMaximum(dur);
        });

    connect(ui.upload, &QPushButton::clicked, this, &AudioSteganography::onUploadButtonClicked);
    connect(ui.embedButton, &QPushButton::clicked, this, &AudioSteganography::onEmbedButtonClicked);
    connect(ui.resetButton, &QPushButton::clicked, this, &AudioSteganography::onResetButtonClicked);
    connect(ui.extractButton, &QPushButton::clicked, this, &AudioSteganography::onExtractButtonClicked);

    scene = new QGraphicsScene(this);
    scene2 = new QGraphicsScene(this);
    ui.audioVisualizerView->setScene(scene);
    ui.audioVisualizerView2->setScene(scene2);

}

AudioSteganography::~AudioSteganography()
{
}

void AudioSteganography::onUploadButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Audio File"), "",
        tr("Audio Files (*.wav);;All Files (*)"));
    if (!fileName.isEmpty()) {
        audioFilePath = fileName.toStdString();

        // Read the audio file and get the waveform data
        AudioData audioData = readAudioFile(audioFilePath.c_str());
        std::vector<double> waveform = prepareWaveformData(audioData.samples, audioData.channels);

        // Clear previous items from the scene
        scene->clear();
        QPen pen(Qt::blue);

        // Downsample the waveform data for visualization
        const size_t waveformDisplaySize = ui.audioVisualizerView->width();
        std::vector<double> downsampledWaveform = downsampleWaveform(waveform, waveformDisplaySize);

        // Make sure we have at least two points to draw a line
        if (downsampledWaveform.size() > 1) {
            int height = ui.audioVisualizerView->height();
            double xScale = static_cast<double>(waveformDisplaySize) / downsampledWaveform.size();
            double verticalScale = height * 0.5; // 50% of the visualizer's height
            for (size_t i = 0; i < downsampledWaveform.size() - 1; ++i) {
                // Draw line segments between points, scaling the x coordinate and using 80% of the height
                scene->addLine(i * xScale, height / 2 - downsampledWaveform[i] * verticalScale,
                    (i + 1) * xScale, height / 2 - downsampledWaveform[i + 1] * verticalScale, pen);
            }
        }
    }
    updateLabelWithFilePath(ui.uploadLabel, fileName);
}


void AudioSteganography::updateLabelWithFilePath(QLabel* label, const QString& filePath) {
    QFontMetrics metrics(label->font());
    int maxWidth = label->width() - 2 * 25; // Maximum width for the text
    QString elidedText = metrics.elidedText(filePath, Qt::ElideMiddle, maxWidth);
    label->setText(elidedText);
}

void AudioSteganography::onEmbedButtonClicked() {
    if (audioFilePath.empty()) {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setWindowTitle(tr("Error"));
        messageBox.setText(tr("Please upload an audio file first."));
        messageBox.setStyleSheet("QMessageBox { background-color: white; color: black; }");
        messageBox.exec();
        return;
    }

    QString qOutputFilePath = QFileDialog::getSaveFileName(this,
        tr("Save Output Audio File"), "",
        tr("WAV Audio (*.wav)"));

    if (!qOutputFilePath.isEmpty()) {
        outputFilePath = qOutputFilePath.toStdString();
    }
    else {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setWindowTitle(tr("Error"));
        messageBox.setText(tr("No output file selected."));
        messageBox.setStyleSheet("QMessageBox { background-color: white; color: black; }");
        messageBox.exec();
        return;
    }

    QString messageQStr = ui.messageText->toPlainText();
    std::string message = messageQStr.toStdString();

    if (hideMessageInAudio(audioFilePath, outputFilePath, message)) {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Information);
        messageBox.setWindowTitle(tr("Success"));
        messageBox.setText(tr("Message hidden successfully."));
        messageBox.setStyleSheet("QMessageBox { background-color: white; color: black; }");
        messageBox.exec();

        // Read the embedded audio file and get the waveform data
        AudioData embeddedAudioData = readAudioFile(outputFilePath.c_str());
        std::vector<double> embeddedWaveform = prepareWaveformData(embeddedAudioData.samples, embeddedAudioData.channels);

        // Read the original audio file again to compare
        AudioData originalAudioData = readAudioFile(audioFilePath.c_str());
        std::vector<double> originalWaveform = prepareWaveformData(originalAudioData.samples, originalAudioData.channels);

        // Clear previous items from the second scene
        scene2->clear();
        QPen penGreen(Qt::green);
        QPen penRed(Qt::red);

        // The width for the visualizer
        const size_t visualizerWidth = ui.audioVisualizerView2->width();

        // Downsample the original waveform data for visualization
        std::vector<double> downsampledOriginalWaveform = downsampleWaveform(originalWaveform, visualizerWidth);

        // Downsample the embedded waveform data for visualization
        std::vector<double> downsampledEmbeddedWaveform = downsampleWaveform(embeddedWaveform, visualizerWidth);

        int height = ui.audioVisualizerView2->height();
        double xScale = static_cast<double>(visualizerWidth) / downsampledEmbeddedWaveform.size();
        double verticalScale = height * 0.5; // 50% of the visualizer's height

        // Draw the waveform on the second visualizer
        if (downsampledEmbeddedWaveform.size() > 1) {
            for (size_t i = 0; i < downsampledEmbeddedWaveform.size() - 1; ++i) {
                bool isDifferent = downsampledOriginalWaveform[i] != downsampledEmbeddedWaveform[i];
                QPen& currentPen = isDifferent ? penRed : penGreen;

                // Draw line segments between points, scaling the x coordinate and using 80% of the height
                scene2->addLine(i * xScale, height / 2 - downsampledEmbeddedWaveform[i] * verticalScale,
                    (i + 1) * xScale, height / 2 - downsampledEmbeddedWaveform[i + 1] * verticalScale, currentPen);
            }
        }
    }
    else {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setWindowTitle(tr("Error"));
        messageBox.setText(tr("Failed to hide the message."));
        messageBox.setStyleSheet("QMessageBox { background-color: white; color: black; }");
        messageBox.exec();
    }
}


std::vector<double> AudioSteganography::downsampleWaveform(const std::vector<double>& waveform, size_t targetSize) {
    std::vector<double> downsampled;
    double downsampleRate = static_cast<double>(waveform.size()) / targetSize;
    for (size_t i = 0; i < targetSize; ++i) {
        size_t index = static_cast<size_t>(i * downsampleRate);
        if (index < waveform.size()) {
            downsampled.push_back(waveform[index]);
        }
    }
    return downsampled;
}

void AudioSteganography::onResetButtonClicked() {
    // Clear the message text
    ui.messageText->clear();

    // Clear the visualizers
    scene->clear();
    scene2->clear();

    // Clear the selected path in the upload label
    ui.uploadLabel->clear();

    // Reset the stored audio file path
    audioFilePath.clear();
    outputFilePath.clear();
}

void AudioSteganography::onExtractButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open WAV File to Extract Message"), "",
        tr("WAV Files (*.wav);;All Files (*)"));

    if (!fileName.isEmpty()) {
        std::string message = audioToMessage(fileName.toStdString());
        ui.messageText->setText(QString::fromStdString(message));
    }
}

void AudioSteganography::onPlayInputButtonClicked() {
    if (inputPlayer->playbackState() == QMediaPlayer::PlayingState) {
        inputPlayer->pause();
    }
    else {
        if (inputPlayer->source() != QUrl::fromLocalFile(QString::fromStdString(audioFilePath))) {
            inputPlayer->setSource(QUrl::fromLocalFile(QString::fromStdString(audioFilePath)));
        }
        inputPlayer->play();
    }
}

void AudioSteganography::onPlayOutputButtonClicked() {
    if (outputPlayer->playbackState() == QMediaPlayer::PlayingState) {
        outputPlayer->pause();
    }
    else {
        if (outputPlayer->source() != QUrl::fromLocalFile(QString::fromStdString(outputFilePath))) {
            outputPlayer->setSource(QUrl::fromLocalFile(QString::fromStdString(outputFilePath)));
        }
        outputPlayer->play();
    }
}

