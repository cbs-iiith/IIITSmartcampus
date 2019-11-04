import os
from celery import Celery

# set the default Django settings module for the 'celery' program.
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'ESW_Campus_Energy_and_Demand_Dashboard.settings')
os.environ.setdefault('FORKED_BY_MULTIPROCESSING', '1')

app = Celery('ESW_Campus_Energy_and_Demand_Dashboard')

app.config_from_object('django.conf:settings', namespace='CELERY')

# Load task modules from all registered Django app configs.
app.autodiscover_tasks()


@app.task(bind=True)
def debug_task(self):
    print('Request: {0!r}'.format(self.request))
