template <class F, class... Args>
bool Ssh::Call(F&& fn, Args... args)
{
	auto rc  = std::forward<F>(fn)(args...);
	if(!WouldBlock(rc) && rc < 0)
		SetError(rc);
	return rc == 0;
}

