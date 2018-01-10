#include "UnloadPlug2w.h"
#include "ui_UnloadPlug2.h"
#include <max.h>

#include <utilapi.h>
#include "iparamb2.h"	// ClassDesc2
#include "plugapi.h"


#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qfiledialog.h>
#include <QtCore\qlibrary.h>
#include <QtCore\qdatetime.h>
#include <Windows.h>
#include <WinBase.h>
#include <time.h>
#include <string.h>
#include <QtCore\QFileInfo>
#include <QtCore\qsettings.h>
#include <QtWidgets\QCheckBox>
#include <QtCore\QDirIterator>


Interface *ip;
QString* filename2;
extern HINSTANCE hInstance;
QList<const DllDesc *> lplugins;
QList<QString > luserplugins;
QList<const DllDesc *> lremovedplugins;
DllDir *dd;
HMODULE dll = 0;
QString SettingsFile = QCoreApplication::applicationDirPath()+"/Plugins/unloadplug2.ini";

UnloadPlug2w::UnloadPlug2w(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::UnloadPlug2_widget)
{
	ui->setupUi(this);
	connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(unloadDll()));
	connect(ui->pushButton_7, SIGNAL(clicked(bool)), this, SLOT(unloadClass()));
	connect(ui->pushButton,   SIGNAL(clicked(bool)), this, SLOT(loadplugin()));
	connect(ui->pushButton_3, SIGNAL(clicked(bool)), this, SLOT(loadlib_n()));
	connect(ui->pushButton_4, SIGNAL(clicked(bool)), this, SLOT(loadlib_nQt()));
	connect(ui->pushButton_5, SIGNAL(clicked(bool)), this, SLOT(loadlib_max()));
	connect(ui->pushButton_6, SIGNAL(clicked(bool)), this, SLOT(about()));
	connect(ui->pushButton_8, SIGNAL(clicked(bool)), this, SLOT(deselectall()));
	connect(ui->checkBox,     SIGNAL(stateChanged(int)), this, SLOT(loadfiles()));
	
    ip = GetCOREInterface();
	dd = ip->GetDllDirectory();
	loadsetting();
}

std::wstring UnloadPlug2w::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void UnloadPlug2w::unloadDll()
{
	for (int i = 0; i < lplugins.count(); i++)
	{
		const DllDesc *myDesc = lplugins.at(i);

		if (checkClassIfExist(myDesc->GetFileName()) == true)
		{
			for (int im = 0; im < myDesc->NumberOfClasses(); im++)
			{
				ClassDesc *class_desc = (*myDesc)[im];
				ip->DeleteClass(class_desc);
			}
			lremovedplugins.append(myDesc);
			FreeLibrary(myDesc->GetHandle());

			QMessageBox msgBox;
			msgBox.setText("Plugin unloaded");
			msgBox.exec();

		}

	}
}

void UnloadPlug2w::unloadClass()
{
	QString ffname= ui->listWidget_2->selectedItems().at(0)->text();
	QFileInfo fi(ffname);
	QLibrary myLib(ffname);

	typedef int(*PluginLibNumberClasses)();
	PluginLibNumberClasses lib_nr = (PluginLibNumberClasses)myLib.resolve("LibNumberClasses");

	typedef ClassDesc* (*PluginLibClassDesc)(int);
	PluginLibClassDesc dec_lib = (PluginLibClassDesc)myLib.resolve("LibClassDesc");

	int test = lib_nr();

	for (int p = 0; p < test; p++)
	{
		ClassDesc* dec = dec_lib(p);
		if (ip->DeleteClass(dec) > 0)
		{
			
			
		}
	}
	for (int i = 0; i < luserplugins.count(); i++)
	{
		if(ui->listWidget_2->selectedItems().at(0)->text()==luserplugins.at(i))
        luserplugins.removeAt(i);
	}
	ui->listWidget_2->selectedItems().removeAt(0);
	myLib.unload();
	QMessageBox msgBox;
	msgBox.setText("Plugin Class unloaded");
	msgBox.exec();
}

void UnloadPlug2w::loadlib_n()
{
	QMessageBox msgBox;
	ui->listWidget->clear();
	lplugins.clear();
	QString ffname = QFileDialog::QFileDialog::getOpenFileName(this,
		tr("Load Plugin"), ".",
		tr("Plugin files (*.dll *.*)"));

	QFileInfo fi(ffname);
	if (luserplugins.contains(ffname) == true)
	{
		msgBox.setText("list already contains this plugin, please unload it to reload again");
		msgBox.exec();
	}
	else
	{
		luserplugins.append(ffname);
		if (!(dll = LoadLibraryA(ffname.toStdString().c_str())))
		{
			msgBox.setText("failed to load plugin");
			msgBox.exec();
		}
		else
		{
			typedef int(*PluginLibNumberClasses)();
			PluginLibNumberClasses lib_nr = (PluginLibNumberClasses)GetProcAddress(dll, "LibNumberClasses");

			typedef ClassDesc* (*PluginLibClassDesc)(int);
			PluginLibClassDesc dec_lib = (PluginLibClassDesc)GetProcAddress(dll, "LibClassDesc");

			typedef int(*PluginLibShutdown)();
			PluginLibShutdown shut_lib = (PluginLibShutdown)GetProcAddress(dll, "LibShutdown");

			typedef ULONG(*PluginLibVersion)();
			PluginLibVersion vers_lib = (PluginLibVersion)GetProcAddress(dll, "LibVersion");

			typedef int(*PluginLibDesc)();
			PluginLibDesc desc_lib = (PluginLibDesc)GetProcAddress(dll, "LibDescription");

			typedef int(*PluginLibInitialize)();
			PluginLibInitialize init_lib = (PluginLibInitialize)GetProcAddress(dll, "LibInitialize");

			init_lib();
			int test = lib_nr();

			for (int p = 0; p < test; p++)
			{
				ClassDesc* dec = dec_lib(p);

				if (dec->SuperClassID() == UTILITY_CLASS_ID)
				{
					UtilityObj * uti = (UtilityObj*)dec->Create();
				}
				if (dec->SuperClassID() == OSM_CLASS_ID)
				{
					Modifier * mod = (Modifier*)dec->Create();
				}
				if (dec->SuperClassID() == RENDERER_CLASS_ID)
				{
					Renderer * ren = (Renderer*)dec->Create();
				}
				if (dec->SuperClassID() == MATERIAL_CLASS_ID)
				{
					Material * mat = (Material*)dec->Create();
				}
				if (dec->SuperClassID() == SHADER_CLASS_ID)
				{
					Shader * sha = (Shader*)dec->Create();
				}
				if (dec->SuperClassID() == TEXMAP_CLASS_ID)
				{
					Texmap * tex = (Texmap*)dec->Create();
				}
				if (dec->SuperClassID() == SCENE_IMPORT_CLASS_ID)
				{
					SceneImport * impo = (SceneImport*)dec->Create();
				}
				if (dec->SuperClassID() == SCENE_EXPORT_CLASS_ID)
				{
					SceneExport * expo = (SceneExport*)dec->Create();
				}
				if (dec->SuperClassID() == GEOMOBJECT_CLASS_ID)
				{
					GeomObject * expo = (GeomObject*)dec->Create();
				}
				if (dec->SuperClassID() == CAMERA_CLASS_ID)
				{
					Camera * cam = (Camera*)dec->Create();
				}
				if (dec->SuperClassID() == LIGHT_CLASS_ID)
				{
					Light * lig = (Light*)dec->Create();
				}
				if (dec->SuperClassID() == SHAPE_CLASS_ID)
				{
					ShapeObject * shap = (ShapeObject*)dec->Create();
				}
				if (dec->SuperClassID() == HELPER_CLASS_ID)
				{
					HelperObject * hel = (HelperObject*)dec->Create();
				}
				if (dec->SuperClassID() == OSM_CLASS_ID)
				{
					Object  * obs = (Object *)dec->Create();
				}
				if (dec->SuperClassID() == WSM_CLASS_ID)
				{
					Modifier* wor = (Modifier*)dec->Create();
				}

				ip->AddClass(dec);

			}
			updatelist();
			updateWidgetlist();
			msgBox.setText("all done");
			msgBox.exec();

		}
	}
}

void UnloadPlug2w::loadlib_n(QString fn)
{
	QMessageBox msgBox;
	ui->listWidget->clear();
	lplugins.clear();
	QString ffname = fn;
	QFileInfo fi(ffname);
	if (luserplugins.contains(ffname) == true)
	{
		msgBox.setText("list already contains this plugin, please unload it to reload again");
		msgBox.exec();
	}
	else
	{
		luserplugins.append(ffname);
		if (!(dll = LoadLibraryA(ffname.toStdString().c_str())))
		{
			msgBox.setText("failed to load plugin");
			msgBox.exec();
		}
		else
		{
			typedef int(*PluginLibNumberClasses)();
			PluginLibNumberClasses lib_nr = (PluginLibNumberClasses)GetProcAddress(dll, "LibNumberClasses");

			typedef ClassDesc* (*PluginLibClassDesc)(int);
			PluginLibClassDesc dec_lib = (PluginLibClassDesc)GetProcAddress(dll, "LibClassDesc");

			typedef int(*PluginLibShutdown)();
			PluginLibShutdown shut_lib = (PluginLibShutdown)GetProcAddress(dll, "LibShutdown");

			typedef ULONG(*PluginLibVersion)();
			PluginLibVersion vers_lib = (PluginLibVersion)GetProcAddress(dll, "LibVersion");

			typedef int(*PluginLibDesc)();
			PluginLibDesc desc_lib = (PluginLibDesc)GetProcAddress(dll, "LibDescription");

			typedef int(*PluginLibInitialize)();
			PluginLibInitialize init_lib = (PluginLibInitialize)GetProcAddress(dll, "LibInitialize");

			init_lib();
			int test = lib_nr();

			for (int p = 0; p < test; p++)
			{
				ClassDesc* dec = dec_lib(p);

				if (dec->SuperClassID() == UTILITY_CLASS_ID)
				{
					UtilityObj * uti = (UtilityObj*)dec->Create();
				}
				if (dec->SuperClassID() == OSM_CLASS_ID)
				{
					Modifier * mod = (Modifier*)dec->Create();
				}
				if (dec->SuperClassID() == RENDERER_CLASS_ID)
				{
					Renderer * ren = (Renderer*)dec->Create();
				}
				if (dec->SuperClassID() == MATERIAL_CLASS_ID)
				{
					Material * mat = (Material*)dec->Create();
				}
				if (dec->SuperClassID() == SHADER_CLASS_ID)
				{
					Shader * sha = (Shader*)dec->Create();
				}
				if (dec->SuperClassID() == TEXMAP_CLASS_ID)
				{
					Texmap * tex = (Texmap*)dec->Create();
				}
				if (dec->SuperClassID() == SCENE_IMPORT_CLASS_ID)
				{
					SceneImport * impo = (SceneImport*)dec->Create();
				}
				if (dec->SuperClassID() == SCENE_EXPORT_CLASS_ID)
				{
					SceneExport * expo = (SceneExport*)dec->Create();
				}
				if (dec->SuperClassID() == GEOMOBJECT_CLASS_ID)
				{
					GeomObject * expo = (GeomObject*)dec->Create();
				}
				if (dec->SuperClassID() == CAMERA_CLASS_ID)
				{
					Camera * cam = (Camera*)dec->Create();
				}
				if (dec->SuperClassID() == LIGHT_CLASS_ID)
				{
					Light * lig = (Light*)dec->Create();
				}
				if (dec->SuperClassID() == SHAPE_CLASS_ID)
				{
					ShapeObject * shap = (ShapeObject*)dec->Create();
				}
				if (dec->SuperClassID() == HELPER_CLASS_ID)
				{
					HelperObject * hel = (HelperObject*)dec->Create();
				}
				if (dec->SuperClassID() == OSM_CLASS_ID)
				{
					Object  * obs = (Object *)dec->Create();
				}
				if (dec->SuperClassID() == WSM_CLASS_ID)
				{
					Modifier* wor = (Modifier*)dec->Create();
				}

				ip->AddClass(dec);

			}
			updatelist();
			updateWidgetlist();
			msgBox.setText("all done");
			msgBox.exec();

		}
	}
}

void UnloadPlug2w::loadlib_nQt()
{
	QMessageBox msgBox;
	ui->listWidget->clear();
	lplugins.clear();
	QString ffname = QFileDialog::QFileDialog::getOpenFileName(this,
		tr("Load Plugin"), ".",
		tr("Plugin files (*.dll *.*)"));

	QFileInfo fi(ffname);
	if (luserplugins.contains(ffname) == true)
	{
		msgBox.setText("list already contains this plugin, please unload it to reload again");
		msgBox.exec();
	}
	else
	{
	luserplugins.append(ffname);
	QLibrary myLib(ffname);

	typedef int(*PluginLibNumberClasses)();
	PluginLibNumberClasses lib_nr = (PluginLibNumberClasses)myLib.resolve("LibNumberClasses");

	typedef ClassDesc* (*PluginLibClassDesc)(int);
	PluginLibClassDesc dec_lib = (PluginLibClassDesc)myLib.resolve("LibClassDesc");

	typedef int(*PluginLibShutdown)();
	PluginLibShutdown shut_lib = (PluginLibShutdown)myLib.resolve("LibShutdown");

	typedef ULONG(*PluginLibVersion)();
	PluginLibVersion vers_lib = (PluginLibVersion)myLib.resolve("LibVersion");

	typedef int(*PluginLibDesc)();
	PluginLibDesc desc_lib = (PluginLibDesc)myLib.resolve("LibDescription");

	typedef int(*PluginLibInitialize)();
	PluginLibInitialize init_lib = (PluginLibInitialize)myLib.resolve("LibInitialize");

	init_lib();
	int test = lib_nr();

	for (int p = 0; p < test; p++)
	{
		ClassDesc* dec = dec_lib(p);

		if (dec->SuperClassID() == UTILITY_CLASS_ID)
		{
			UtilityObj * uti = (UtilityObj*)dec->Create();
		}
		if (dec->SuperClassID() == OSM_CLASS_ID)
		{
			Modifier * mod = (Modifier*)dec->Create();
		}
		if (dec->SuperClassID() == RENDERER_CLASS_ID)
		{
			Renderer * ren = (Renderer*)dec->Create();
		}
		if (dec->SuperClassID() == MATERIAL_CLASS_ID)
		{
			Material * mat = (Material*)dec->Create();
		}
		if (dec->SuperClassID() == SHADER_CLASS_ID)
		{
			Shader * sha = (Shader*)dec->Create();
		}
		if (dec->SuperClassID() == TEXMAP_CLASS_ID)
		{
			Texmap * tex = (Texmap*)dec->Create();
		}
		if (dec->SuperClassID() == SCENE_IMPORT_CLASS_ID)
		{
			SceneImport * impo = (SceneImport*)dec->Create();
		}
		if (dec->SuperClassID() == SCENE_EXPORT_CLASS_ID)
		{
			SceneExport * expo = (SceneExport*)dec->Create();
		}
		if (dec->SuperClassID() == GEOMOBJECT_CLASS_ID)
		{
			GeomObject * expo = (GeomObject*)dec->Create();
		}
		if (dec->SuperClassID() == CAMERA_CLASS_ID)
		{
			Camera * cam = (Camera*)dec->Create();
		}
		if (dec->SuperClassID() == LIGHT_CLASS_ID)
		{
			Light * lig = (Light*)dec->Create();
		}
		if (dec->SuperClassID() == SHAPE_CLASS_ID)
		{
			ShapeObject * shap = (ShapeObject*)dec->Create();
		}
		if (dec->SuperClassID() == HELPER_CLASS_ID)
		{
			HelperObject * hel = (HelperObject*)dec->Create();
		}
		if (dec->SuperClassID() == OSM_CLASS_ID)
		{
			Object  * obs = (Object *)dec->Create();
		}
		if (dec->SuperClassID() == WSM_CLASS_ID)
		{
			Modifier* wor = (Modifier*)dec->Create();
		}

		ip->AddClass(dec);

	}
	updatelist();
	updateWidgetlist();
	myLib.unload();
	msgBox.setText("all done");
	msgBox.exec();
	}
}

void UnloadPlug2w::loadlib_max()
{
	ui->listWidget->clearSelection();
	QString ffname = QFileDialog::QFileDialog::getOpenFileName(this,
		tr("Load Plugin"), ".",
		tr("Plugin files (*.dll *.*)"));

	dd->LoadADll(converToWChar_t(ffname), true);
	QMessageBox msgBox;
	msgBox.setText("All done");
	msgBox.exec();
	
}

bool UnloadPlug2w::checkClassIfExist(QString descClassname)
{
	QString lw = ui->listWidget->selectedItems().at(0)->text();
	if (descClassname == lw)
	{
		return true;
	}
	else
	{
		return false;
	}
}

UnloadPlug2w::~UnloadPlug2w()
{
	delete ui;
}

wchar_t* UnloadPlug2w::converToWChar_t(QString text)
{
	wchar_t * c_Text = new wchar_t[text.length() + 1];
	text.toWCharArray(c_Text);
	c_Text[text.length()] = 0;
	return c_Text;
}

void UnloadPlug2w::loadplugin()
{
	updatelist();
	updateWidgetlist();
}

QString replacedir(QString path)
{
	QString temp=path.replace("/","\\");
	return temp;
}

void UnloadPlug2w::updatelist()
{
	for (int i = 0; i < dd->Count(); i++)
	{
		const DllDesc *myDesc = &(*dd)[i];
		int pluginDllSizeKb = myDesc->GetSize() / 1024;
		if (myDesc->IsLoaded())
		{
			
			if (lremovedplugins.contains(myDesc))
			{

			}
			else
			{
             lplugins.append(myDesc);
			}
		}
		
	}
   
}

void UnloadPlug2w::updateWidgetlist()
{
	for (int i = 0; i < lplugins.count(); i++)
		{
			const DllDesc *myDesc = lplugins.at(i);
			QListWidgetItem *item = new QListWidgetItem();
			item->setText(myDesc->GetFileName());
			ui->listWidget->addItem(item);
		}

	for (int p = 0; p < luserplugins.count(); p++)
	{
		QListWidgetItem *item = new QListWidgetItem();
		item->setText(luserplugins.at(p));
		ui->listWidget_2->addItem(item);
	}
}

void UnloadPlug2w::about()
{
	QString info = "for this plugin to work u need ,LibInitialize, LibShutdown,LibDescription,LibNumberClasses,LibVersion,LibClassDesc,inside file,extension(dll,dlu,blah blah) of the file dosnt matter" ;

	QMessageBox msgBox;
	msgBox.setText("Plugin Description");
	msgBox.setDetailedText(info);
	msgBox.setStandardButtons(QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();
	switch (ret)
	{
	case QMessageBox::Cancel:
		// do nothing
		break;
	default:
		// should never be reached
		break;
	}

	
}

void UnloadPlug2w::deselectall()
{
	ui->listWidget->clearSelection();
	ui->listWidget_2->clearSelection();
}

void UnloadPlug2w::loadsetting()
{

	QMessageBox msgBox;
	if (QFile(SettingsFile).exists()==true)
	{
		QSettings settings(SettingsFile, QSettings::IniFormat);
		bool sets = settings.value("LoadOptions/Autoload").toBool();
		ui->checkBox->setChecked(sets);
	}
	else
	{		
		msgBox.setDetailedText(SettingsFile);
		msgBox.setText("Settings file dosnt exist");
		msgBox.exec();
	}
}

void UnloadPlug2w::savesetting()
{
	QSettings settings(SettingsFile, QSettings::IniFormat);
	settings.beginGroup("LoadOptions");
	settings.setValue("LoadOptions/Autoload", ui->checkBox->isChecked());
	settings.endGroup();
}

void UnloadPlug2w::loadfiles()
{
	if(QDir(QCoreApplication::applicationDirPath() + "/Plugins/unloadplugPlugins").exists()==false)
		QDir().mkdir(QCoreApplication::applicationDirPath() + "/Plugins/unloadplugPlugins");
	
	if (ui->checkBox->isChecked() == true)
	{
		QSettings settings(SettingsFile, QSettings::IniFormat);
		QString sets = settings.value("LoadOptions/PluginFolder").toString();
		QString plugpath = QCoreApplication::applicationDirPath() + "/Plugins" + sets;
		QDirIterator it(plugpath, QDir::Files);
		while (it.hasNext())
		{

			QFile f(it.next());
			loadlib_n(f.fileName());
		}
	}
}

