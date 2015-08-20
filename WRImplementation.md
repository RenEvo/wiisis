# Files #

  * Core\WR\_Implementation.h
  * Core\WR\_Implementation.cpp

# Description #

The Implementation files include all of the interface header files used by the library. Several macros are also safely defined to ensure the library compiles and executes successfully.

The **CWR\_GlobalInstance** class is a singleton class. It can be accessed in three ways:
```
CWR_GlobalInstance *pWR = CWR_GlobalInstance::GetInstance();
CWR_GlobalInstance *pWR = GetWiiRemoteSystem(); // Global function
CWR_GlobalInstance *pWR = g_pWR; // Externalized reference
```

You should call **Initialize** when the application boots up to prepare the underlaying sub-modules. Somewhere within the main game loop, you will need to call **Update** _once per frame_ to update the sub-modules. This is crucial; failing to do this will cause everything including the Wii Remote objects to not update. When the application terminates, you will need to call **Shutdown** to clean everything up.

Its listener will notify you if an error occurs in any of the sub-modules.