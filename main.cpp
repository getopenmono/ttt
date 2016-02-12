#include <mono.h>
#include "app_controller.h"

int main()
{
    AppController app;
    mono::IApplicationContext::Instance->setMonoApplication(&app);
    app.enterRunLoop();
	return 0;
}
