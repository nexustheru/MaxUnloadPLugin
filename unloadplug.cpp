

#include "common.h"
#include "resource.h"

#include <max.h>

#include <iparamb2.h>
#include <utilapi.h>
#include <plugin.h>
#include <windowsx.h>
#include "UnloadPlug2w.h"

// Handle to this plugin's module.
extern HINSTANCE hInstance;

 
class UnloadPlug2 : public UtilityObj 
{
public:
	virtual ~UnloadPlug2() {};
  virtual void BeginEditParams(Interface *ip, IUtil * /*iu*/) {
	  UnloadPlug2::ip = ip;
	  up2 = new UnloadPlug2w();	  
	  ip->AddRollupPage(*up2, L"Load Unload Plugin");
	 
  }
  virtual void EndEditParams(Interface *ip, IUtil * /*iu*/) { ip->DeleteRollupPage(*up2); }
  // Implement this ourselves to ensure consistent heap usage.
  virtual void DeleteThis() { ip->DeleteRollupPage(*up2); }
  virtual void Init(HWND hWnd) {};
  virtual void Destroy(HWND hWnd) {};

static UnloadPlug2* GetInstance()
{
	static UnloadPlug2 theUnloadPlug2;
	return &theUnloadPlug2;
}
private:
  HWND unload_rollup_;
  Interface *ip;
  UnloadPlug2w* up2;
};

// Class descriptor for our plugin.
class UnloadPlug2Desc : public ClassDesc2 {
public:

  virtual int IsPublic() { return TRUE;}
  virtual void *Create(BOOL /*loading = FALSE*/) {return UnloadPlug2::GetInstance(); }
  virtual const TCHAR *ClassName() { return _T("Unload Plugin");}
  virtual SClass_ID SuperClassID() {return UTILITY_CLASS_ID;}
  virtual Class_ID ClassID() {return UNLOAD_PLUG2_CLASS_ID;}
  virtual const TCHAR *Category() {return _T("");}
  virtual const TCHAR* InternalName() { return _T("Unload Plugin"); }	// returns fixed parsable name (scripter-visible name)
  virtual HINSTANCE HInstance() { return hInstance; }
};

static UnloadPlug2Desc unload_plug2_desc;

// Returns a singleton instance of our class descriptor.
ClassDesc *GetUnloadPlug2Desc() {
  return &unload_plug2_desc;
}