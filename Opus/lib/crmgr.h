/*
	CRMGR.H : CoRoutine Manager Exports
*/

/* Procedure Prototypes */
typedef DWORD (FAR PASCAL *PFN_CRMGR)();
typedef WORD	HSTACK;

#ifdef CC
/* routines callable from main application */
HSTACK		FAR PASCAL HstackMyData(void);
VOID		FAR PASCAL CreateStack(HSTACK, WORD *);
DWORD		FAR PASCAL LCallOtherStack(PFN_CRMGR, HSTACK, WORD *, WORD);
#endif /*CC*/

#define	WCallOtherStack(lpfn, hstack, rgw, cw)	\
	((WORD)LCallOtherStack(lpfn, hstack, rgw, cw))
#define	CallOtherStack(lpfn, hstack, rgw, cw)	\
	((VOID)LCallOtherStack(lpfn, hstack, rgw, cw))


