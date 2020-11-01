#include "algorithm_thread.h"
#include "enforce.h"
#include <Windows.h>
#include <atomic>
#include <new>
#pragma comment(lib, "Synchronization.lib")

// TODO: This synchronization is probably overkill: Maybe remove atomics or reduce the number of generation counters...

extern std::atomic<bool> should_continue_global;
static std::atomic<bool> should_exit_algorithm;

static sort_function_pointer sort_function;
static HANDLE thread_handle;

static bool is_run_all;
static std::atomic<bool> paused;	//Rarely modified: cache line segreggation isn't required.
static std::atomic<uint32_t> head;	//Same
static std::atomic<uint32_t> tail;	//Same

static DWORD WINAPI thread_entry_point(void* unused) noexcept
{
	auto& main_array = *(::main_array*)nullptr;
	while (should_continue_global.load(std::memory_order_acquire))
	{
		auto h = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&head, &h, sizeof(head), INFINITE);
		if (should_exit_algorithm.load(std::memory_order_acquire))
			break;
		if (sort_function == nullptr)
			continue;
		sort_function(main_array);
		sort_function = nullptr;
		(void)tail.fetch_add(1, std::memory_order_release);
		(void)WakeByAddressSingle(&tail);
	}
	return 0;
}

namespace algorithm_thread
{
	void assign_body(sort_function_pointer sort) noexcept
	{
		await();
		sort_function = sort;
		signal();
	}

	void launch() noexcept
	{
		thread_handle = CreateThread(nullptr, 1 << 21, thread_entry_point, nullptr, 0, nullptr);
		enforce(thread_handle != nullptr);
	}

	bool is_paused() noexcept
	{
		return paused.load(std::memory_order_acquire);
	}

	bool is_idle() noexcept
	{
		return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
	}

	void pause() noexcept
	{
		SuspendThread(thread_handle);
		paused.store(true, std::memory_order_release);
	}

	void resume() noexcept
	{
		ResumeThread(thread_handle);
		paused.store(false, std::memory_order_release);
	}

	void abort_sort() noexcept
	{
		should_exit_algorithm.store(true, std::memory_order_release);
		(void)head.fetch_add(1, std::memory_order_release);
		(void)WakeByAddressSingle(&head);
		if (WaitForSingleObject(thread_handle, 10) == WAIT_TIMEOUT)
			TerminateThread(thread_handle, 0); //Return 0, nothing happened here :^)
		sort_function = nullptr;
		non_atomic_store(should_exit_algorithm, false);
		non_atomic_store(paused, false);
		non_atomic_store(head, 0);
		non_atomic_store(tail, 0);
		launch();
	}

	void signal() noexcept
	{
		(void)head.fetch_add(1, std::memory_order_acquire);
		WakeByAddressSingle(&head);
	}

	void await(uint32_t timeout_ms) noexcept
	{
		if (is_idle())
			return;
		auto desired = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&tail, &desired, sizeof(desired), timeout_ms == UINT32_MAX ? INFINITE : timeout_ms);
	}

	void terminate() noexcept
	{
		(void)TerminateThread(thread_handle, 0);
	}
}