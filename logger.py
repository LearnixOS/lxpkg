import logging
from logging.handlers import RotatingFileHandler
import os
from config import LOG_DIR

def setup_logger():
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    logger = logging.getLogger('lxpkg')
    logger.setLevel(logging.DEBUG)

    handler = RotatingFileHandler(os.path.join(LOG_DIR, 'lxpkg.log'), maxBytes=10485760, backupCount=5)
    handler.setLevel(logging.DEBUG)

    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)

    logger.addHandler(handler)

    return logger

logger = setup_logger()

