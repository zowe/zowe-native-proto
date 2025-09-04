"""
Routes package for the Flask application.
Contains modularized route blueprints for different z/OS service endpoints.
"""

from .zds import zds_bp
from .zusf import zusf_bp
from .zjb import zjb_bp

__all__ = ['zds_bp', 'zusf_bp', 'zjb_bp']