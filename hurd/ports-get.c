static error_t
getbootstrap (mach_port_t *result)
{
  return __task_get_special_port (__mach_task_self (),
				  TASK_BOOTSTRAP_PORT,
				  result);
}

error_t (*_hurd_ports_getters[INIT_PORT_MAX]) (mach_port_t *result) =
  {
    [INIT_PORT_BOOTSTRAP] = getbootstrap
  }

error_t
_hurd_ports_get (int which, mach_port_t *result)
{
  if (which < 0 || which >= _hurd_nports)
    return EINVAL;
  if (which >= INIT_PORT_MAX || _hurd_ports_getters[which] == NULL)
    return HURD_PORT_USE (&_hurd_ports[which],
			  __mach_port_mod_refs (__mach_task_self (),
						(*result = port),
						MACH_PORT_RIGHT_SEND,
						+1));
  return (*_hurd_ports_getters[which]) (result);
}
