/*
 Copyright (c) TrueCrypt Foundation. All rights reserved.

 Covered by the TrueCrypt License 2.3 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include <guiddef.h>

template <class TClass>
class TrueCryptFactory : public IClassFactory
{

public:
	TrueCryptFactory (DWORD messageThreadId) : 
		RefCount (1), ServerLockCount (0), MessageThreadId (messageThreadId) { }

	~TrueCryptFactory () { }
	
	virtual ULONG STDMETHODCALLTYPE AddRef ()
	{
		return InterlockedIncrement (&RefCount) - 1;
	}

	virtual ULONG STDMETHODCALLTYPE Release ()
	{
		ULONG r = InterlockedDecrement (&RefCount) + 1;

		if (r == 0)
			delete this;

		return r;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void **ppvObject)
	{
		if (riid == IID_IUnknown || riid == IID_IClassFactory)
			*ppvObject = this;
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		AddRef ();
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE CreateInstance (IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
	{
		if (pUnkOuter != NULL)
			return CLASS_E_NOAGGREGATION;

		TClass *tc = new TClass (MessageThreadId);
		if (tc == NULL)
			return E_OUTOFMEMORY;

		HRESULT hr = tc->QueryInterface (riid, ppvObject);

		if (hr)
			delete tc;

		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE LockServer (BOOL fLock)
	{
		if (fLock)
		{
			InterlockedIncrement (&ServerLockCount);
		}
		else
		{
			if (!InterlockedDecrement (&ServerLockCount))
				PostThreadMessage (MessageThreadId, WM_APP, 0, 0);
		}

		return S_OK;
	}

	virtual bool IsServerLocked ()
	{
		return ServerLockCount > 0;
	}

protected:
	DWORD MessageThreadId;
	LONG RefCount;
	LONG ServerLockCount;
};


BOOL ComGetInstanceBase (HWND hWnd, REFCLSID clsid, REFIID iid, void **tcServer);
HRESULT CreateElevatedComObject (HWND hwnd, REFGUID guid, REFIID iid, void **ppv);
