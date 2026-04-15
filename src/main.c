
#include "Utils.h"
#include "app.h"
#include "state_machine.h"



int main() {
    App_Init();

    while (1) {
        SM_Update();
        delay_ms(100);
    }
    return 0;
}
