from django.db import models
from django.utils  import timezone


class MainModel(models.Model):
	temp = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	humidity = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	watts_total = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	watts_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	watts_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	watts_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	frequency = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	pf_avg = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	pf_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	pf_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	pf_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	v_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	v_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	v_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	current_total = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	current_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	current_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	current_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	var_total = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	var_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	var_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	var_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	va_total = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	va_r_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	va_b_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	va_y_phase = models.DecimalField(decimal_places=3, max_digits=7, default=0)
	time = models.DateTimeField(default=timezone.now)

	def __str__(self):
		return str(self.pk)
