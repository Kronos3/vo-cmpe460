#ifndef VO_CMPE460_DBG_BUTTON_H
#define VO_CMPE460_DBG_BUTTON_H

#include <circle/types.h>
#include <circle/gpiopin.h>

#include <dbg/dbg_act.h>

class Button
{
    CGPIOPin m_pin;
public:
    explicit Button(u32 pin, CGPIOManager* gpio_mgr);
    void Initialize();

protected:
    virtual void Interrupt() = 0;
    static void InterruptHandler(void* param);
};

class DbgButton : public Button
{
    DbgAct* m_dbg_act;
    volatile boolean m_blocked;

public:
    explicit DbgButton(u32 pin,
                       DbgAct* dbg_act,
                       CGPIOManager* gpio_mgr);
    void Breakpoint(u32 n = 0);

private:
    void Interrupt() override;
};

class ExecuteButton : public Button
{
public:
    ExecuteButton(u32 pin, CGPIOManager* gpio_mgr,
                  void (*function)());
private:
    void (*m_function)();
    void Interrupt() override;
};

#endif //VO_CMPE460_DBG_BUTTON_H
