#pragma once
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QTimer>
#include <atomic>
#include <memory> //for std::unique_ptr

namespace Ui {
class MainWindow;
}
namespace lsl {
class stream_outlet;
}
struct TobiiResearchEyeTracker;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent, const char *config_file);
	~MainWindow() noexcept override;

	std::unique_ptr<lsl::stream_outlet> gaze_stream;

private slots:
	void closeEvent(QCloseEvent *ev) override;
	void toggleRecording();
	void refresh_eyetrackers();
	void refresh_samplingrate();

private:
	// function for loading / saving the config file
	QString find_config_file(const char *filename);
	void load_config(const QString &filename);
	void save_config(const QString &filename);
	std::unique_ptr<Ui::MainWindow> ui; // window pointer
	TobiiResearchEyeTracker *current_tracker;
	QTimer statusTimer;
};
