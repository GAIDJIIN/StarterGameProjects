// Florist Game. All rights reserved.


#include "SaveGameFunctionLibrary.h"

bool USaveGameFunctionLibrary::WasObjectLoaded(UObject* Object)
{
	return Object && Object->HasAnyFlags(RF_WasLoaded | RF_LoadCompleted);
}
