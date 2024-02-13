#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AudioSteganography.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>
#include <QMediaPlayer>


class AudioSteganography : public QMainWindow
{
    Q_OBJECT

public:
    AudioSteganography(QWidget *parent = nullptr);
    ~AudioSteganography();

private slots:
    void onEmbedButtonClicked();
	void onUploadButtonClicked();
    void updateLabelWithFilePath(QLabel* label, const QString& filePath);
    std::vector<double> downsampleWaveform(const std::vector<double>& waveform, size_t targetSize);
    void onResetButtonClicked();
    void onExtractButtonClicked();
    void onPlayInputButtonClicked();
    void onPlayOutputButtonClicked();

private:
    Ui::AudioSteganographyClass ui;
    std::string audioFilePath;
    QGraphicsScene* scene;
    QGraphicsScene* scene2;
    QMediaPlayer* inputPlayer;
    QMediaPlayer* outputPlayer;
    QSlider* inputAudioSlider;
    QSlider* outputAudioSlider;
    std::string outputFilePath; 
};

