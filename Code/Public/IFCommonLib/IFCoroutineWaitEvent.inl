
static void WaitEvent(IFEventSlot<void()>& e )
{
	IFRefPtr<IFCoroutine> spCurCor = getCurrentCoroutine();
    printf("%p :%p=%s\n", spCurCor.getPtr(), spCurCor->m_sName.c_str(),spCurCor->m_sName.c_str());
	IFEventSlot<void()>* pEvent = &e;
	e.AddSelfHoldHandleL([=]()
	{
		IFRefPtr<IFCoroutine> spCor = spCurCor;
		printf("%p :%p=%s\n", spCor.getPtr(), spCor->m_sName.c_str(),spCor->m_sName.c_str());
		spCor->resume();
		pEvent->RemoveSelfHoldHandle(spCor->m_sName);

	}, spCurCor->m_sName);
	yied();
}

template<typename P0>
static void WaitEvent(IFEventSlot<void(P0)>& e, P0* pP0 )
{
	IFRefPtr<IFCoroutine> spCurCor = getCurrentCoroutine();
	IFEventSlot<void(P0)>* pEvent = &e;
	e.AddSelfHoldHandleL([=](P0 p0)
	{
		*pP0 = p0;
		IFRefPtr<IFCoroutine> spCor = spCurCor;
		spCor->resume();
		pEvent->RemoveSelfHoldHandle(spCor->m_sName);

	}, spCurCor->m_sName);
	yied();
}

template<typename P0, typename P1>
static void WaitEvent(IFEventSlot<void(P0,P1)>& e, P0* pP0, P1* pP1 )
{
	IFRefPtr<IFCoroutine> spCurCor = getCurrentCoroutine();
	IFEventSlot<void(P0,P1)>* pEvent = &e;
	e.AddSelfHoldHandleL([=](P0 p0, P1 p1)
	{
		*pP0 = p0;
		*pP1 = p1;
		IFRefPtr<IFCoroutine> spCor = spCurCor;
		spCor->resume();
		pEvent->RemoveSelfHoldHandle(spCor->m_sName);

	}, spCurCor->m_sName);
	yied();
}
	

