#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <TobiiLib.h>
#include <chrono>
#include <cmath>
#include <lsl_cpp.h>
#include <tobii_research.h>
#include <tobii_research_eyetracker.h>
#include <tobii_research_streams.h>

static const char tobiisdklicense[] =
	"1) You may not copy (except for backup purposes), modify, adapt, decompile, reverse \
engineer, disassemble, or create derivative works of the Software Components or any part thereof.\n\
2) You may not redistribute or combine/bundle any part of the Software Components \
with other software, or distribute any software or device incorporating part of the Software \
Components.\n\
3) You shall agree that Tobii (i) owns all legal right, title and interest in and to the \
Software Components, including any related intellectual property rights; and (ii) reserves all \
rights not expressly granted.\n\
4) You shall agree that You have no right to use any of Tobii's trade names, \
trademarks, service marks, logos, domain names, or other distinctive brand features.\n\
5) You will not remove, obscure, or alter any proprietary rights \
notices (including copyright and trademark notices) that may be affixed to or contained within \
the Software Components";

template <typename Args>
QString tobii_str_wrap_QString(TobiiResearchStatus (*funcptr)(Args, char **), Args args) {
	char *tmp;
	if (auto res = funcptr(args, &tmp) != TOBII_RESEARCH_STATUS_OK)
		throw std::runtime_error("Error in Tobii Call: " + std::to_string(res));
	QString res(tmp);
	tobii_research_free_string(tmp);
	return res;
}

static uint32_t samples_written = 0;

void push_tobii_gaze(TobiiResearchGazeData *g, void *gaze_stream) {
	auto gs = reinterpret_cast<lsl::stream_outlet *>(gaze_stream);
	if (!g->left_eye.gaze_point.validity) {
		g->left_eye.gaze_point.position_on_display_area.x = NAN;
		g->left_eye.gaze_point.position_on_display_area.y = NAN;
	}
	if (!g->right_eye.gaze_point.validity) {
		g->right_eye.gaze_point.position_on_display_area.x = NAN;
		g->right_eye.gaze_point.position_on_display_area.y = NAN;
	}
	const float sample[] = {g->device_time_stamp,
		g->system_time_stamp,
		g->left_eye.gaze_point.position_on_display_area.x,
		g->left_eye.gaze_point.position_on_display_area.y,
		g->left_eye.pupil_data.validity ? g->left_eye.pupil_data.diameter : NAN,
		g->right_eye.gaze_point.position_on_display_area.x,
		g->right_eye.gaze_point.position_on_display_area.y,
		g->right_eye.pupil_data.validity ? g->right_eye.pupil_data.diameter : NAN};
	gs->push_sample(sample);
	samples_written++;
};

MainWindow::MainWindow(QWidget *parent, const char *config_file)
	: QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	connect(ui->address, &QLineEdit::textChanged, this, &MainWindow::refresh_samplingrate);
	connect(ui->actionLoad_Configuration, &QAction::triggered, [this]() {
		load_config(QFileDialog::getOpenFileName(
			this, "Load Configuration File", "", "Configuration Files (*.cfg)"));
	});
	connect(ui->actionSave_Configuration, &QAction::triggered, [this]() {
		save_config(QFileDialog::getSaveFileName(
			this, "Save Configuration File", "", "Configuration Files (*.cfg)"));
	});
	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actionAbout, &QAction::triggered,
		[this]() { QMessageBox::about(this, "LSL TobiiPro App", tobiisdklicense); });
	QObject::connect(ui->findButton, &QPushButton::clicked, this, &MainWindow::refresh_eyetrackers);
	connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

	QString cfgfilepath = find_config_file(config_file);
	load_config(cfgfilepath);

	statusTimer.setInterval(1000);
	connect(&statusTimer, &QTimer::timeout, [this]() {
		this->ui->statusBar->showMessage("Samples sent: " + QString::number(samples_written));
	});
}

void MainWindow::load_config(const QString &filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	ui->address->setText(settings.value("TobiiPro/address", "127.0.0.1").toString());
	ui->samplingrate->setText(settings.value("TobiiPro/rate", "60").toString());
	ui->participant->setText(settings.value("Participant/ID", "P0").toString());
}

void MainWindow::save_config(const QString &filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("TobiiPro");
	settings.setValue("address", ui->address->text());
	settings.setValue("rate", ui->samplingrate->text());
	settings.endGroup();
	settings.beginGroup("Participant");
	settings.setValue("ID", ui->participant->text());
	settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *ev) {
	if (gaze_stream) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}

void MainWindow::refresh_eyetrackers() {
	TobiiResearchEyeTrackers *eyetrackers;
	if (auto res = tobii_research_find_all_eyetrackers(&eyetrackers)) {
		QMessageBox::warning(
			this, "Error", "Could not enumerate eye trackers: " + QString::number(res));
		return;
	}

	ui->trackerDropdown->clear();
	for (std::size_t i = 0; i < eyetrackers->count; ++i) {
		ui->trackerDropdown->addItem(
			tobii_str_wrap_QString(&tobii_research_get_address, eyetrackers->eyetrackers[i]));
	}
}

void MainWindow::refresh_samplingrate() {
	ui->samplingrateDropdown->clear();
	QString address = ui->address->text();
	try {
		
		QByteArray addr = address.toLocal8Bit();
		auto res = tobii_research_get_eyetracker(addr.constData(), &current_tracker);
		if (res != TOBII_RESEARCH_STATUS_OK)
			return;
		
		TobiiResearchGazeOutputFrequencies* frequencies = NULL;
		size_t i = 0;
		int status = tobii_research_get_all_gaze_output_frequencies(current_tracker, &frequencies);
		for (; i < frequencies->frequency_count; i++) {
			ui->samplingrateDropdown->addItem(QString::number(frequencies->frequencies[i]));
		}
	}
	catch (std::exception & e) {
		QMessageBox::critical(
			this, "Error", QStringLiteral("Error: ") + e.what(), QMessageBox::Ok);
	}

	
}

void MainWindow::toggleRecording() {
	if (!gaze_stream) {
		ui->linkButton->setEnabled(false);
		// === perform link action ===
		QString participant = ui->participant->text();
		QString address = ui->address->text();
		QString ratestr = ui->samplingrate->text();
		bool ok = false;
		double rate = ratestr.toDouble(&ok);

		try {
			QByteArray addr = address.toLocal8Bit();
			auto res = tobii_research_get_eyetracker(addr.constData(), &current_tracker);
			if (res != TOBII_RESEARCH_STATUS_OK)
				throw std::runtime_error("Could not connect: " + std::to_string(res));

			// Start LSL outlet
			std::string streamname = "Tobii";
			lsl::stream_info info(streamname+"-"+participant.toLocal8Bit().constData(), "Eyetracker", 8,rate, lsl::cf_float32,
				streamname + "at_" + address.toStdString());

			// append some meta-data
			info.desc()
				.append_child("acquisition")
				.append_child_value("manufacturer", "TobiiPro")
				.append_child_value("model", "TODO")
				.append_child_value("application", "TobiiProApp")
				.append_child_value("precision", "24");

			auto channels = info.desc().append_child("channels");
			channels.append_child("channel")
				.append_child_value("label", "device_ts")
				.append_child_value("type", "timestamp")
				.append_child_value("unit", "milliseconds");
			channels.append_child("channel")
				.append_child_value("label", "system_ts")
				.append_child_value("type", "timestamp")
				.append_child_value("unit", "milliseconds");
			channels.append_child("channel")
				.append_child_value("label", "left_x")
				.append_child_value("eye", "left")
				.append_child_value("type", "ScreenX")
				.append_child_value("unit", "screencoords");
			channels.append_child("channel")
				.append_child_value("label", "left_y")
				.append_child_value("eye", "left")
				.append_child_value("type", "ScreenY")
				.append_child_value("unit", "screencoords");
			channels.append_child("channel")
				.append_child_value("label", "left_pupil")
				.append_child_value("eye", "left")
				.append_child_value("type", "pupilsize");
			channels.append_child("channel")
				.append_child_value("label", "right_x")
				.append_child_value("eye", "right")
				.append_child_value("type", "ScreenX")
				.append_child_value("unit", "screencoords");
			channels.append_child("channel")
				.append_child_value("label", "right_y")
				.append_child_value("eye", "right")
				.append_child_value("type", "ScreenY")
				.append_child_value("unit", "screencoords");
			channels.append_child("channel")
				.append_child_value("label", "right_pupil")
				.append_child_value("eye", "right")
				.append_child_value("type", "pupilsize");

			// make a new outlet
			gaze_stream.reset(new lsl::stream_outlet(info, 1200));
			qInfo() << "Created outlet";

			// reset the counter
			samples_written = 0;

			tobii_research_set_gaze_output_frequency(current_tracker, rate);
			// subscribe to the stream
			res = tobii_research_subscribe_to_gaze_data(
				current_tracker, push_tobii_gaze, gaze_stream.get());
			qInfo() << "Subscribed: " << res;

			const int sync_samples = 10;
			for (int i = 0; i < sync_samples; ++i) {
				int64_t tobii_clock;
				tobii_research_get_system_time_stamp(&tobii_clock);
				double lsl_clock = lsl::local_clock();
				qInfo() << "Difference LSL clock - Tobii Clock"
						<< (static_cast<int64_t>(lsl_clock * 1000000) - tobii_clock);
			}
			statusTimer.start();
		} catch (std::exception &e) {
			QMessageBox::critical(
				this, "Error", QStringLiteral("Error: ") + e.what(), QMessageBox::Ok);
		}
		ui->linkButton->setEnabled(true);
		ui->linkButton->setText("Stop stream");
	} else {
		// === perform unlink action ===
		ui->linkButton->setEnabled(false);
		tobii_research_unsubscribe_from_gaze_data(current_tracker, push_tobii_gaze);
		gaze_stream.reset();
		statusTimer.stop();
		ui->linkButton->setEnabled(true);
		ui->linkButton->setText("Start stream");
	}
}

/**
 * Find a config file to load. This is (in descending order or preference):
 * - a file supplied on the command line
 * - [executablename].cfg in one the the following folders:
 *	- the current working directory
 *	- the default config folder, e.g. '~/Library/Preferences' on OS X
 *	- the executable folder
 * @param filename	Optional file name supplied e.g. as command line parameter
 * @return Path to a found config file
 */
QString MainWindow::find_config_file(const char *filename) {
	if (filename) {
		QString qfilename(filename);
		if (!QFileInfo::exists(qfilename))
			QMessageBox(QMessageBox::Warning, "Config file not found",
				QStringLiteral("The file '%1' doesn't exist").arg(qfilename), QMessageBox::Ok,
				this);
		else
			return qfilename;
	}
	QFileInfo exeInfo(QCoreApplication::applicationFilePath());
	QString defaultCfgFilename(exeInfo.completeBaseName() + ".cfg");
	QStringList cfgpaths;
	cfgpaths << QDir::currentPath()
			 << QStandardPaths::standardLocations(QStandardPaths::ConfigLocation) << exeInfo.path();
	for (auto path : cfgpaths) {
		QString cfgfilepath = path + QDir::separator() + defaultCfgFilename;
		if (QFileInfo::exists(cfgfilepath)) return cfgfilepath;
	}
	QMessageBox(QMessageBox::Warning, "No config file not found",
		QStringLiteral("No default config file could be found"), QMessageBox::Ok, this);
	return "";
}


MainWindow::~MainWindow() noexcept = default;
