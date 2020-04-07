#include <windows.h>
#include <cassert>
#include <cstdio>
#include <iostream>

struct message_box_parameters
{
  LPCWSTR lp_text;
  LPCWSTR lp_cap;
};

class message_box_impl
{
 public:
  ~message_box_impl();

  __declspec(noinline) virtual void show(LPVOID p)
  {
    const auto wnd = GetForegroundWindow();
    const message_box_parameters *mbp = (struct message_box_parameters *)(LPVOID)p;
    MessageBoxW(wnd, mbp->lp_text, mbp->lp_cap, MB_OKCANCEL);
  };
};

message_box_impl::~message_box_impl() = default;

int main()
{
  DWORD t1_id;
  //
  wchar_t lp_text[] = L"Text";
  wchar_t lp_cap[] = L"Title";
  message_box_parameters parameters{lp_text, lp_cap};
  //
  const auto sync = CreateMutex(nullptr, FALSE, nullptr);
  WaitForSingleObject(sync, INFINITE);
  auto stack_protect = [](LPVOID p1) -> DWORD
  {
    DWORD t2_id;

    const auto sync2 = CreateMutex(nullptr, FALSE, nullptr);

    WaitForSingleObject(sync2, INFINITE);

    auto show_message_box = [](LPVOID p2) -> DWORD
    {
      message_box_impl base_obj;
      message_box_impl *p = &base_obj;
      p->show(p2);
      return 0;
    };

    const auto t2 = CreateThread(nullptr, 0, show_message_box, p1, 0, &t2_id);

    ReleaseMutex(sync2);

    if (t2)
    {
      CloseHandle(t2);
    }

    if (sync2)
    {
      CloseHandle(sync2);
    }

    return 0;
  };
  const auto t1 = CreateThread(nullptr, 0, stack_protect, &parameters, 0, &t1_id);
  ReleaseMutex(sync);
  if (t1)
  {
    CloseHandle(t1);
  }
  if (sync)
  {
    CloseHandle(sync);
  }
  getchar();
}
