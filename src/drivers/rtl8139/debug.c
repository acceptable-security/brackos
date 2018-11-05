#include <drivers/rtl8139.h>
#include <kprint.h>

void rtl8139_debug_tsd(int i, rtl8139_tsd_t tsd) {
	kprintf("rtl8139: tsd%d (%x): %s%s%s%s%s%s%sertxth = %d ncc = %d\n",
		i,
		tsd.data,
		tsd.own ? "own " : "",
		tsd.tun ? "tun " : "",
		tsd.tok ? "tok " : "",
		tsd.cdh ? "cdh " : "",
		tsd.owc ? "owc " : "",
		tsd.tabt ? "tabt " : "",
		tsd.crs ? "crs " : "",
		tsd.ertxth,
		tsd.ncc);
}