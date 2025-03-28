#pragma once

#include <condition_variable>
#include <mutex>

class ExecutionContext
{
public:
	void SetResult(const int &p_tResult)
	{
		{
			std::lock_guard<std::mutex> lock{m_oResultMutex};
			m_tResult = p_tResult;
			m_bFinished = true;
		}
		m_oResultConditionVariable.notify_one();
	}
	int Wait()
	{
		std::unique_lock<std::mutex> lock{m_oResultMutex};
		m_oResultConditionVariable.wait(lock, [this]()
										{ return m_bFinished; });
		return m_tResult;
	}

private:
	std::mutex m_oResultMutex;
	std::condition_variable m_oResultConditionVariable;
	int m_tResult;
	bool m_bFinished{false};
};
