#pragma once

#include <QtWidgets\qwidget.h>
#include <QtCore\qobject.h>
#include <QtWidgets\qlistwidget.h>

namespace Ui
{
	class UnloadPlug2_widget;
};

class UnloadPlug2w : public QWidget
{
	// Connects us to Qt's meta-object system
	Q_OBJECT

public:
	explicit UnloadPlug2w(QWidget* parent = 0);

	~UnloadPlug2w();
	wchar_t* converToWChar_t(QString text);
	std::wstring s2ws(const std::string& s);

	bool checkClassIfExist(QString descClassname);
	public slots:
	void loadplugin();
	void updatelist();
	void updateWidgetlist();
	void loadlib_n();
	void loadlib_nQt();
	void loadlib_max();
	void unloadDll();
	void unloadClass();
	void about();
	void deselectall();
	void loadsetting();
	void savesetting();

private:
	Ui::UnloadPlug2_widget* ui;
	QString SettingsFile="unloadplug2.ini";
};