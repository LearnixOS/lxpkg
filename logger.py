import logging
from logging.handlers import RotatingFileHandler
import os

# Define a default LOG_DIR in case config.py import fails
DEFAULT_LOG_DIR = os.path.expanduser('~/.config/lxpkg/logs')

# Try to import LOG_DIR from config.py, fallback if it fails
try:
    from config import LOG_DIR
except ModuleNotFoundError:
    LOG_DIR = DEFAULT_LOG_DIR
    print(f"Warning: Could not import LOG_DIR from config.py. Using default: {LOG_DIR}")

def setup_logger():
    """Set up the logger with file rotation."""
    # Use a local variable to ensure LOG_DIR is always defined in this scope
    log_dir = LOG_DIR

    # Ensure log_dir exists
    try:
        os.makedirs(log_dir, exist_ok=True)
    except Exception as e:
        # Fallback to a different directory if the original fails
        fallback_log_dir = os.path.expanduser('~/lxpkg_logs')
        try:
            os.makedirs(fallback_log_dir, exist_ok=True)
            print(f"Error creating log directory {log_dir}: {e}. Falling back to {fallback_log_dir}")
            log_dir = fallback_log_dir
        except Exception as e2:
            print(f"Error creating fallback log directory {fallback_log_dir}: {e2}. Logging to file disabled.")
            log_dir = None  # Disable file logging if both attempts fail

    # Create logger
    logger = logging.getLogger('lxpkg')
    logger.setLevel(logging.DEBUG)  # Capture all levels, handlers will filter

    # Only set up file handler if we have a valid log_dir
    if log_dir:
        # Create rotating file handler
        log_file = os.path.join(log_dir, 'lxpkg.log')
        handler = RotatingFileHandler(
            log_file,
            maxBytes=10*1024*1024,  # 10 MB
            backupCount=5  # Keep 5 backup files
        )
        handler.setLevel(logging.DEBUG)  # Log all levels to file

        # Define log format
        formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        handler.setFormatter(formatter)

        # Clear any existing handlers to avoid duplicates
        if logger.handlers:
            logger.handlers.clear()

        # Add handler to logger
        logger.addHandler(handler)

    return logger

# Create and export the logger instance
logger = setup_logger()
