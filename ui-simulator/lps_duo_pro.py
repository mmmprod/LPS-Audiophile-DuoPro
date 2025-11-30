#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LPS DUO PRO - Interface Utilisateur PyGame V92
Simulateur PC pour développement et tests

IMPORTANT: Ce code PyGame est un SIMULATEUR PC pour le développement.
L'interface production tourne sur ESP32-8048S050C avec écran LCD 5" intégré
(800x480) et communique avec un ATmega328P pour la régulation.

Architecture du système:
- ESP32-8048S050C : Écran 5" 800x480, Interface LVGL, Tactile capacitif
- ATmega328P : Régulation LDO, Protections, Mesures ADC
- Communication UART entre ESP32 et ATmega

Optimisations V92:
- draw.lines() pour oscilloscopes (690->3 appels/frame)
- Cache get_all_problems() par frame
- Layout centralisé (ECOUTE_LAYOUT, DETAILS_LAYOUT, etc.)
- LCD agrandi 95px
- Headroom 2.0V conforme circuit V2.4.5

Version: 92
Auteur: LPS Audiophile Team
"""

import pygame
import math
import time
import random
import os
import sys
from datetime import datetime, timedelta
from typing import Dict, List, Tuple, Optional, Any, Callable
from dataclasses import dataclass, field
from enum import Enum, auto

# =============================================================================
# CONFIGURATION AUDIO ENGINE
# =============================================================================

# Tentative d'import sounddevice pour audio avancé
AUDIO_ENGINE = "pygame"  # Default fallback
try:
    import sounddevice as sd
    import numpy as np
    AUDIO_ENGINE = "sounddevice"
except ImportError:
    try:
        import numpy as np
    except ImportError:
        np = None

# =============================================================================
# CONSTANTES GLOBALES
# =============================================================================

# Dimensions écran (ESP32-8048S050C: 800x480)
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 480

# FPS cible
TARGET_FPS = 30

# Couleurs thème audiophile
class Colors:
    """Palette de couleurs du thème audiophile"""
    BLACK = (0, 0, 0)
    WHITE = (255, 255, 255)
    DARK_GRAY = (30, 30, 35)
    MID_GRAY = (60, 60, 70)
    LIGHT_GRAY = (120, 120, 130)
    
    # Couleurs principales
    AMBER = (255, 176, 0)
    AMBER_DIM = (180, 120, 0)
    AMBER_DARK = (100, 70, 0)
    
    GREEN = (0, 255, 100)
    GREEN_DIM = (0, 180, 70)
    GREEN_DARK = (0, 100, 40)
    
    RED = (255, 50, 50)
    RED_DIM = (180, 35, 35)
    RED_DARK = (100, 20, 20)
    
    BLUE = (50, 150, 255)
    BLUE_DIM = (35, 100, 180)
    BLUE_DARK = (20, 60, 100)
    
    CYAN = (0, 255, 255)
    CYAN_DIM = (0, 180, 180)
    
    ORANGE = (255, 120, 0)
    YELLOW = (255, 255, 0)
    PURPLE = (180, 100, 255)
    
    # Couleurs VU-mètre
    VU_GREEN = (0, 200, 0)
    VU_YELLOW = (255, 200, 0)
    VU_RED = (255, 0, 0)
    
    # Fond LCD
    LCD_BG = (10, 15, 20)
    LCD_BORDER = (40, 50, 60)

# =============================================================================
# FONT BITMAP 8x8 PERSONNALISÉE
# =============================================================================

class BitmapFont:
    """Police bitmap 8x8 personnalisée pour affichage rétro"""
    
    # Caractères 8x8 en format binaire (chaque ligne = 8 bits)
    CHARS = {
        '0': [0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00],
        '1': [0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00],
        '2': [0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00],
        '3': [0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00],
        '4': [0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C, 0x00],
        '5': [0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00],
        '6': [0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00],
        '7': [0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00],
        '8': [0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00],
        '9': [0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00],
        'A': [0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00],
        'B': [0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00],
        'C': [0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00],
        'D': [0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00],
        'E': [0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00],
        'F': [0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00],
        'G': [0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00],
        'H': [0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00],
        'I': [0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00],
        'J': [0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00],
        'K': [0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00],
        'L': [0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00],
        'M': [0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00],
        'N': [0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00],
        'O': [0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00],
        'P': [0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00],
        'Q': [0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00],
        'R': [0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00],
        'S': [0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00],
        'T': [0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00],
        'U': [0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00],
        'V': [0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00],
        'W': [0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00],
        'X': [0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00],
        'Y': [0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00],
        'Z': [0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00],
        ' ': [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
        '.': [0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00],
        ':': [0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00],
        '-': [0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00],
        '+': [0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00],
        '/': [0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00],
        '%': [0x62, 0x64, 0x08, 0x10, 0x26, 0x46, 0x00, 0x00],
        '(': [0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00],
        ')': [0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00],
        '<': [0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00],
        '>': [0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00],
        '=': [0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00],
        '_': [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00],
        '!': [0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00],
        '?': [0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00],
        '#': [0x24, 0x7E, 0x24, 0x24, 0x7E, 0x24, 0x00, 0x00],
        '*': [0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00],
        '@': [0x3C, 0x66, 0x6E, 0x6A, 0x6E, 0x60, 0x3C, 0x00],
    }
    
    # Caractères minuscules (mapping vers majuscules pour simplicité)
    for c in 'abcdefghijklmnopqrstuvwxyz':
        CHARS[c] = CHARS[c.upper()]
    
    @classmethod
    def render_char(cls, surface: pygame.Surface, char: str, x: int, y: int, 
                    color: Tuple[int, int, int], scale: int = 1) -> int:
        """Rend un caractère à la position donnée, retourne la largeur"""
        if char not in cls.CHARS:
            char = '?'
        
        bitmap = cls.CHARS[char]
        for row_idx, row in enumerate(bitmap):
            for col_idx in range(8):
                if row & (0x80 >> col_idx):
                    if scale == 1:
                        surface.set_at((x + col_idx, y + row_idx), color)
                    else:
                        pygame.draw.rect(surface, color, 
                                        (x + col_idx * scale, y + row_idx * scale, 
                                         scale, scale))
        return 8 * scale
    
    @classmethod
    def render_text(cls, surface: pygame.Surface, text: str, x: int, y: int,
                   color: Tuple[int, int, int], scale: int = 1, spacing: int = 1) -> int:
        """Rend une chaîne de texte, retourne la largeur totale"""
        total_width = 0
        for char in text:
            width = cls.render_char(surface, char, x + total_width, y, color, scale)
            total_width += width + spacing * scale
        return total_width - spacing * scale if text else 0
    
    @classmethod
    def get_text_width(cls, text: str, scale: int = 1, spacing: int = 1) -> int:
        """Calcule la largeur d'un texte sans le dessiner"""
        if not text:
            return 0
        return len(text) * 8 * scale + (len(text) - 1) * spacing * scale


# =============================================================================
# SYSTÈME DE TRADUCTIONS
# =============================================================================

class Language(Enum):
    """Langues supportées"""
    FR = "Français"
    EN = "English"
    ES = "Español"
    DE = "Deutsch"


class Translations:
    """Système de traductions multi-langues"""
    
    STRINGS = {
        # Pages principales
        "page_listen": {"FR": "ÉCOUTE", "EN": "LISTEN", "ES": "ESCUCHA", "DE": "HÖREN"},
        "page_details": {"FR": "DÉTAILS", "EN": "DETAILS", "ES": "DETALLES", "DE": "DETAILS"},
        "page_health": {"FR": "SANTÉ", "EN": "HEALTH", "ES": "SALUD", "DE": "STATUS"},
        "page_session": {"FR": "SESSION", "EN": "SESSION", "ES": "SESIÓN", "DE": "SITZUNG"},
        "page_config": {"FR": "CONFIG", "EN": "CONFIG", "ES": "CONFIG", "DE": "CONFIG"},
        
        # Labels communs
        "rail_a": {"FR": "RAIL A", "EN": "RAIL A", "ES": "RAIL A", "DE": "KANAL A"},
        "rail_b": {"FR": "RAIL B", "EN": "RAIL B", "ES": "RAIL B", "DE": "KANAL B"},
        "voltage": {"FR": "TENSION", "EN": "VOLTAGE", "ES": "VOLTAJE", "DE": "SPANNUNG"},
        "current": {"FR": "COURANT", "EN": "CURRENT", "ES": "CORRIENTE", "DE": "STROM"},
        "power": {"FR": "PUISSANCE", "EN": "POWER", "ES": "POTENCIA", "DE": "LEISTUNG"},
        "temperature": {"FR": "TEMPÉRATURE", "EN": "TEMPERATURE", "ES": "TEMPERATURA", "DE": "TEMPERATUR"},
        "headroom": {"FR": "RÉSERVE", "EN": "HEADROOM", "ES": "RESERVA", "DE": "RESERVE"},
        "ripple": {"FR": "ONDULATION", "EN": "RIPPLE", "ES": "RIZADO", "DE": "WELLIGKEIT"},
        "noise": {"FR": "BRUIT", "EN": "NOISE", "ES": "RUIDO", "DE": "RAUSCHEN"},
        "efficiency": {"FR": "RENDEMENT", "EN": "EFFICIENCY", "ES": "EFICIENCIA", "DE": "EFFIZIENZ"},
        
        # États
        "ok": {"FR": "OK", "EN": "OK", "ES": "OK", "DE": "OK"},
        "warning": {"FR": "ATTENTION", "EN": "WARNING", "ES": "ALERTA", "DE": "WARNUNG"},
        "error": {"FR": "ERREUR", "EN": "ERROR", "ES": "ERROR", "DE": "FEHLER"},
        "active": {"FR": "ACTIF", "EN": "ACTIVE", "ES": "ACTIVO", "DE": "AKTIV"},
        "standby": {"FR": "VEILLE", "EN": "STANDBY", "ES": "ESPERA", "DE": "BEREIT"},
        "off": {"FR": "ARRÊT", "EN": "OFF", "ES": "APAGADO", "DE": "AUS"},
        
        # Protections
        "ovp": {"FR": "SURTENSION", "EN": "OVERVOLTAGE", "ES": "SOBRETENSIÓN", "DE": "ÜBERSPANNUNG"},
        "ocp": {"FR": "SURINTENSITÉ", "EN": "OVERCURRENT", "ES": "SOBRECORRIENTE", "DE": "ÜBERSTROM"},
        "otp": {"FR": "SURCHAUFFE", "EN": "OVERTEMP", "ES": "SOBRETEMPERATURA", "DE": "ÜBERTEMPERATUR"},
        "protection_active": {"FR": "PROTECTION ACTIVE", "EN": "PROTECTION ACTIVE", 
                             "ES": "PROTECCIÓN ACTIVA", "DE": "SCHUTZ AKTIV"},
        
        # Session
        "uptime": {"FR": "DURÉE", "EN": "UPTIME", "ES": "TIEMPO", "DE": "LAUFZEIT"},
        "energy": {"FR": "ÉNERGIE", "EN": "ENERGY", "ES": "ENERGÍA", "DE": "ENERGIE"},
        "session_start": {"FR": "DÉBUT SESSION", "EN": "SESSION START", 
                         "ES": "INICIO SESIÓN", "DE": "SITZUNGSSTART"},
        
        # Config
        "language": {"FR": "LANGUE", "EN": "LANGUAGE", "ES": "IDIOMA", "DE": "SPRACHE"},
        "brightness": {"FR": "LUMINOSITÉ", "EN": "BRIGHTNESS", "ES": "BRILLO", "DE": "HELLIGKEIT"},
        "auto_off": {"FR": "ARRÊT AUTO", "EN": "AUTO OFF", "ES": "APAGADO AUTO", "DE": "AUTO AUS"},
        "simulation": {"FR": "SIMULATION", "EN": "SIMULATION", "ES": "SIMULACIÓN", "DE": "SIMULATION"},
        "reset": {"FR": "RÉINITIALISER", "EN": "RESET", "ES": "REINICIAR", "DE": "ZURÜCKSETZEN"},
        "about": {"FR": "À PROPOS", "EN": "ABOUT", "ES": "ACERCA DE", "DE": "ÜBER"},
        
        # Simulations de pannes
        "sim_normal": {"FR": "NORM", "EN": "NORM", "ES": "NORM", "DE": "NORM"},
        "sim_hot": {"FR": "HOT", "EN": "HOT", "ES": "HOT", "DE": "HEISS"},
        "sim_ripple": {"FR": "RIP", "EN": "RIP", "ES": "RIP", "DE": "RIP"},
        "sim_load": {"FR": "LOAD", "EN": "LOAD", "ES": "CARGA", "DE": "LAST"},
        "sim_lo_v": {"FR": "LO-V", "EN": "LO-V", "ES": "LO-V", "DE": "LO-V"},
        "sim_hi_v": {"FR": "HI-V", "EN": "HI-V", "ES": "HI-V", "DE": "HI-V"},
        
        # Aide
        "help_title": {"FR": "AIDE", "EN": "HELP", "ES": "AYUDA", "DE": "HILFE"},
        "help_nav": {"FR": "Navigation: Touches 1-5", "EN": "Navigation: Keys 1-5",
                    "ES": "Navegación: Teclas 1-5", "DE": "Navigation: Tasten 1-5"},
        "help_esc": {"FR": "ESC: Fermer popup", "EN": "ESC: Close popup",
                    "ES": "ESC: Cerrar popup", "DE": "ESC: Popup schließen"},
        
        # Unités
        "unit_v": {"FR": "V", "EN": "V", "ES": "V", "DE": "V"},
        "unit_ma": {"FR": "mA", "EN": "mA", "ES": "mA", "DE": "mA"},
        "unit_w": {"FR": "W", "EN": "W", "ES": "W", "DE": "W"},
        "unit_c": {"FR": "°C", "EN": "°C", "ES": "°C", "DE": "°C"},
        "unit_uv": {"FR": "µV", "EN": "µV", "ES": "µV", "DE": "µV"},
        "unit_wh": {"FR": "Wh", "EN": "Wh", "ES": "Wh", "DE": "Wh"},
        "unit_percent": {"FR": "%", "EN": "%", "ES": "%", "DE": "%"},
    }
    
    _current_lang = Language.FR
    
    @classmethod
    def set_language(cls, lang: Language):
        """Change la langue actuelle"""
        cls._current_lang = lang
    
    @classmethod
    def get(cls, key: str) -> str:
        """Récupère une traduction pour la langue actuelle"""
        if key in cls.STRINGS:
            lang_code = cls._current_lang.name
            if lang_code in cls.STRINGS[key]:
                return cls.STRINGS[key][lang_code]
            # Fallback to English
            return cls.STRINGS[key].get("EN", key)
        return key
    
    @classmethod
    def get_current_language(cls) -> Language:
        """Retourne la langue actuelle"""
        return cls._current_lang
    
    @classmethod
    def next_language(cls):
        """Passe à la langue suivante"""
        langs = list(Language)
        idx = langs.index(cls._current_lang)
        cls._current_lang = langs[(idx + 1) % len(langs)]


# Alias pour faciliter l'usage
T = Translations.get


# =============================================================================
# SIMULATION DE DONNÉES
# =============================================================================

class SimulationMode(Enum):
    """Modes de simulation de pannes"""
    NORMAL = auto()
    HOT = auto()      # Surchauffe (OTP)
    RIPPLE = auto()   # Bruit excessif
    LOAD = auto()     # Surcharge (OCP)
    LOW_V = auto()    # Sous-tension
    HIGH_V = auto()   # Surtension (OVP)


@dataclass
class RailData:
    """Données d'un rail d'alimentation"""
    voltage_target: float = 12.0
    voltage_actual: float = 12.0
    current_ma: float = 150.0
    power_w: float = 1.8
    temperature_c: float = 35.0
    headroom_v: float = 2.0
    ripple_uv: float = 5.0
    noise_uv: float = 3.0
    efficiency: float = 92.0
    enabled: bool = True
    
    # États de protection
    ovp_active: bool = False
    ocp_active: bool = False
    otp_active: bool = False


@dataclass  
class SystemData:
    """Données système globales"""
    rail_a: RailData = field(default_factory=RailData)
    rail_b: RailData = field(default_factory=RailData)
    input_voltage: float = 24.0
    ambient_temp: float = 25.0
    uptime_seconds: int = 0
    energy_wh: float = 0.0
    session_start: datetime = field(default_factory=datetime.now)
    simulation_mode: SimulationMode = SimulationMode.NORMAL
    
    # Cache pour optimisation V92
    _problems_cache: List[str] = field(default_factory=list)
    _problems_cache_frame: int = -1


class DataSimulator:
    """Générateur de données simulées"""
    
    def __init__(self):
        self.data = SystemData()
        self.data.rail_a.voltage_target = 12.0
        self.data.rail_b.voltage_target = 5.0
        self.frame_count = 0
        self._oscilloscope_data_a: List[float] = []
        self._oscilloscope_data_b: List[float] = []
        self._init_oscilloscope_data()
    
    def _init_oscilloscope_data(self):
        """Initialise les données d'oscilloscope"""
        for _ in range(200):
            self._oscilloscope_data_a.append(0.0)
            self._oscilloscope_data_b.append(0.0)
    
    def update(self, dt: float):
        """Met à jour les données simulées"""
        self.frame_count += 1
        self.data.uptime_seconds += int(dt)
        
        mode = self.data.simulation_mode
        
        # Simulation normale avec variations réalistes
        noise_a = random.gauss(0, 0.01)
        noise_b = random.gauss(0, 0.01)
        
        # Rail A
        if mode == SimulationMode.NORMAL:
            self.data.rail_a.voltage_actual = self.data.rail_a.voltage_target + noise_a
            self.data.rail_a.current_ma = 150 + random.gauss(0, 5)
            self.data.rail_a.temperature_c = 35 + random.gauss(0, 1)
            self.data.rail_a.ripple_uv = 5 + random.gauss(0, 1)
        elif mode == SimulationMode.HOT:
            self.data.rail_a.temperature_c = 90 + random.gauss(0, 2)
            self.data.rail_a.otp_active = True
        elif mode == SimulationMode.RIPPLE:
            self.data.rail_a.ripple_uv = 100 + random.gauss(0, 20)
        elif mode == SimulationMode.LOAD:
            self.data.rail_a.current_ma = 600 + random.gauss(0, 20)
            self.data.rail_a.ocp_active = True
        elif mode == SimulationMode.LOW_V:
            self.data.rail_a.voltage_actual = self.data.rail_a.voltage_target * 0.8
        elif mode == SimulationMode.HIGH_V:
            self.data.rail_a.voltage_actual = 17.0 + random.gauss(0, 0.2)
            self.data.rail_a.ovp_active = True
        
        # Rail B (similaire)
        self.data.rail_b.voltage_actual = self.data.rail_b.voltage_target + noise_b
        self.data.rail_b.current_ma = 100 + random.gauss(0, 3)
        self.data.rail_b.temperature_c = 32 + random.gauss(0, 1)
        
        # Calcul puissance et énergie
        self.data.rail_a.power_w = (self.data.rail_a.voltage_actual * 
                                     self.data.rail_a.current_ma / 1000)
        self.data.rail_b.power_w = (self.data.rail_b.voltage_actual * 
                                     self.data.rail_b.current_ma / 1000)
        self.data.energy_wh += (self.data.rail_a.power_w + self.data.rail_b.power_w) * dt / 3600
        
        # Update oscilloscope data
        self._oscilloscope_data_a.pop(0)
        self._oscilloscope_data_a.append(self.data.rail_a.voltage_actual)
        self._oscilloscope_data_b.pop(0)
        self._oscilloscope_data_b.append(self.data.rail_b.voltage_actual)
    
    def get_oscilloscope_points(self, rail: str, width: int, height: int, 
                                 x_offset: int, y_offset: int) -> List[Tuple[int, int]]:
        """Retourne les points pour draw.lines() - Optimisation V92"""
        data = self._oscilloscope_data_a if rail == 'A' else self._oscilloscope_data_b
        target = self.data.rail_a.voltage_target if rail == 'A' else self.data.rail_b.voltage_target
        
        points = []
        for i, v in enumerate(data[-width:]):
            x = x_offset + i
            # Normaliser autour de la cible
            deviation = (v - target) * 50  # Amplifier la déviation
            y = y_offset + height // 2 - int(deviation)
            y = max(y_offset, min(y_offset + height, y))
            points.append((x, y))
        return points
    
    def get_all_problems(self) -> List[str]:
        """Retourne la liste des problèmes - avec cache V92"""
        if self.data._problems_cache_frame == self.frame_count:
            return self.data._problems_cache
        
        problems = []
        if self.data.rail_a.ovp_active:
            problems.append("OVP Rail A")
        if self.data.rail_a.ocp_active:
            problems.append("OCP Rail A")
        if self.data.rail_a.otp_active:
            problems.append("OTP Rail A")
        if self.data.rail_b.ovp_active:
            problems.append("OVP Rail B")
        if self.data.rail_b.ocp_active:
            problems.append("OCP Rail B")
        if self.data.rail_b.otp_active:
            problems.append("OTP Rail B")
        if self.data.rail_a.ripple_uv > 50:
            problems.append("Ripple élevé A")
        if self.data.rail_b.ripple_uv > 50:
            problems.append("Ripple élevé B")
        
        self.data._problems_cache = problems
        self.data._problems_cache_frame = self.frame_count
        return problems
    
    def set_simulation_mode(self, mode: SimulationMode):
        """Change le mode de simulation"""
        # Reset des états
        self.data.rail_a.ovp_active = False
        self.data.rail_a.ocp_active = False
        self.data.rail_a.otp_active = False
        self.data.rail_b.ovp_active = False
        self.data.rail_b.ocp_active = False
        self.data.rail_b.otp_active = False
        self.data.simulation_mode = mode


# =============================================================================
# LAYOUTS CENTRALISÉS V92
# =============================================================================

# Layout page ÉCOUTE
ECOUTE_LAYOUT = {
    "title_y": 10,
    "gauge_a": {"x": 50, "y": 60, "width": 300, "height": 180},
    "gauge_b": {"x": 450, "y": 60, "width": 300, "height": 180},
    "lcd_y": 260,
    "lcd_height": 95,
    "status_y": 380,
}

# Layout page DÉTAILS
DETAILS_LAYOUT = {
    "title_y": 10,
    "rail_a_x": 20,
    "rail_b_x": 410,
    "metrics_y": 50,
    "bar_height": 20,
    "spacing": 35,
}

# Layout page SANTÉ
HEALTH_LAYOUT = {
    "title_y": 10,
    "status_y": 50,
    "protection_y": 150,
    "temp_y": 280,
}

# Layout page SESSION
SESSION_LAYOUT = {
    "title_y": 10,
    "timer_y": 80,
    "energy_y": 180,
    "stats_y": 280,
}

# Layout page CONFIG
CONFIG_LAYOUT = {
    "title_y": 10,
    "options_y": 60,
    "option_height": 45,
    "sim_buttons_y": 350,
}


# =============================================================================
# COMPOSANTS UI
# =============================================================================

class VUMeter:
    """Jauge VU-mètre style analogique"""
    
    def __init__(self, x: int, y: int, width: int, height: int):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.value = 0.0
        self.peak = 0.0
        self.peak_decay = 0.95
    
    def update(self, value: float, target: float):
        """Met à jour la valeur et le peak"""
        # Normaliser 0-1 basé sur la cible
        self.value = min(1.0, max(0.0, value / target)) if target > 0 else 0
        if self.value > self.peak:
            self.peak = self.value
        else:
            self.peak *= self.peak_decay
    
    def draw(self, surface: pygame.Surface):
        """Dessine le VU-mètre"""
        # Fond
        pygame.draw.rect(surface, Colors.DARK_GRAY, 
                        (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, Colors.LCD_BORDER,
                        (self.x, self.y, self.width, self.height), 2)
        
        # Segments colorés
        segments = 20
        seg_width = (self.width - 10) // segments
        seg_height = self.height - 20
        
        for i in range(segments):
            # Couleur basée sur la position
            if i < segments * 0.6:
                color = Colors.VU_GREEN
                dim_color = Colors.GREEN_DARK
            elif i < segments * 0.8:
                color = Colors.VU_YELLOW
                dim_color = Colors.AMBER_DARK
            else:
                color = Colors.VU_RED
                dim_color = Colors.RED_DARK
            
            # Segment allumé ou éteint
            threshold = i / segments
            if self.value > threshold:
                seg_color = color
            else:
                seg_color = dim_color
            
            sx = self.x + 5 + i * seg_width
            sy = self.y + 10
            pygame.draw.rect(surface, seg_color, 
                           (sx, sy, seg_width - 2, seg_height))
        
        # Indicateur peak
        if self.peak > 0:
            peak_x = self.x + 5 + int(self.peak * (self.width - 10))
            pygame.draw.line(surface, Colors.WHITE,
                           (peak_x, self.y + 5), (peak_x, self.y + self.height - 5), 2)


class NixieBar:
    """Barre de progression style Nixie tube"""
    
    @staticmethod
    def draw(surface: pygame.Surface, x: int, y: int, width: int, height: int,
             value: float, max_value: float, color: Tuple[int, int, int] = Colors.AMBER):
        """Dessine une barre Nixie"""
        # Fond
        pygame.draw.rect(surface, Colors.DARK_GRAY, (x, y, width, height))
        pygame.draw.rect(surface, Colors.MID_GRAY, (x, y, width, height), 1)
        
        # Remplissage
        fill_width = int((value / max_value) * (width - 4)) if max_value > 0 else 0
        fill_width = max(0, min(width - 4, fill_width))
        
        if fill_width > 0:
            # Dégradé simulé avec segments
            segments = fill_width // 3
            for i in range(segments):
                intensity = 0.7 + 0.3 * (i / max(1, segments - 1))
                seg_color = tuple(int(c * intensity) for c in color)
                pygame.draw.rect(surface, seg_color,
                               (x + 2 + i * 3, y + 2, 2, height - 4))


class Button:
    """Bouton tactile simulé"""
    
    def __init__(self, x: int, y: int, width: int, height: int, 
                 text: str, callback: Optional[Callable] = None):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.callback = callback
        self.pressed = False
        self.hover = False
    
    def handle_event(self, event: pygame.event.Event) -> bool:
        """Gère les événements, retourne True si cliqué"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            if self.rect.collidepoint(event.pos):
                self.pressed = True
                return False
        elif event.type == pygame.MOUSEBUTTONUP:
            if self.pressed and self.rect.collidepoint(event.pos):
                self.pressed = False
                if self.callback:
                    self.callback()
                return True
            self.pressed = False
        elif event.type == pygame.MOUSEMOTION:
            self.hover = self.rect.collidepoint(event.pos)
        return False
    
    def draw(self, surface: pygame.Surface, font: pygame.font.Font):
        """Dessine le bouton"""
        if self.pressed:
            color = Colors.AMBER
            text_color = Colors.BLACK
        elif self.hover:
            color = Colors.MID_GRAY
            text_color = Colors.AMBER
        else:
            color = Colors.DARK_GRAY
            text_color = Colors.AMBER
        
        pygame.draw.rect(surface, color, self.rect)
        pygame.draw.rect(surface, Colors.AMBER, self.rect, 2)
        
        text_surf = font.render(self.text, True, text_color)
        text_rect = text_surf.get_rect(center=self.rect.center)
        surface.blit(text_surf, text_rect)


# =============================================================================
# PAGES DE L'INTERFACE
# =============================================================================

class BasePage:
    """Classe de base pour les pages"""
    
    def __init__(self, app: 'LPSDuoProApp'):
        self.app = app
    
    def update(self, dt: float):
        """Mise à jour logique"""
        pass
    
    def draw(self, surface: pygame.Surface):
        """Rendu graphique"""
        pass
    
    def handle_event(self, event: pygame.event.Event):
        """Gestion des événements"""
        pass


class PageEcoute(BasePage):
    """Page ÉCOUTE - Jauges VU-mètre principales"""
    
    def __init__(self, app: 'LPSDuoProApp'):
        super().__init__(app)
        layout = ECOUTE_LAYOUT
        self.vu_a = VUMeter(layout["gauge_a"]["x"], layout["gauge_a"]["y"],
                           layout["gauge_a"]["width"], layout["gauge_a"]["height"])
        self.vu_b = VUMeter(layout["gauge_b"]["x"], layout["gauge_b"]["y"],
                           layout["gauge_b"]["width"], layout["gauge_b"]["height"])
    
    def update(self, dt: float):
        data = self.app.simulator.data
        self.vu_a.update(data.rail_a.voltage_actual, data.rail_a.voltage_target)
        self.vu_b.update(data.rail_b.voltage_actual, data.rail_b.voltage_target)
    
    def draw(self, surface: pygame.Surface):
        layout = ECOUTE_LAYOUT
        data = self.app.simulator.data
        
        # Titre
        title = T("page_listen")
        title_surf = self.app.font_large.render(title, True, Colors.AMBER)
        surface.blit(title_surf, (SCREEN_WIDTH // 2 - title_surf.get_width() // 2, 
                                  layout["title_y"]))
        
        # Labels rails
        label_a = f"{T('rail_a')}: {data.rail_a.voltage_actual:.2f}V"
        label_b = f"{T('rail_b')}: {data.rail_b.voltage_actual:.2f}V"
        
        surf_a = self.app.font_medium.render(label_a, True, Colors.GREEN)
        surf_b = self.app.font_medium.render(label_b, True, Colors.CYAN)
        
        surface.blit(surf_a, (layout["gauge_a"]["x"], layout["gauge_a"]["y"] - 25))
        surface.blit(surf_b, (layout["gauge_b"]["x"], layout["gauge_b"]["y"] - 25))
        
        # VU-mètres
        self.vu_a.draw(surface)
        self.vu_b.draw(surface)
        
        # Zone LCD avec oscilloscope - Optimisation V92 avec draw.lines()
        lcd_rect = pygame.Rect(50, layout["lcd_y"], 700, layout["lcd_height"])
        pygame.draw.rect(surface, Colors.LCD_BG, lcd_rect)
        pygame.draw.rect(surface, Colors.LCD_BORDER, lcd_rect, 2)
        
        # Oscilloscope Rail A (gauche)
        points_a = self.app.simulator.get_oscilloscope_points(
            'A', 300, 80, 70, layout["lcd_y"] + 8)
        if len(points_a) > 1:
            pygame.draw.lines(surface, Colors.GREEN, False, points_a, 1)
        
        # Oscilloscope Rail B (droite)  
        points_b = self.app.simulator.get_oscilloscope_points(
            'B', 300, 80, 430, layout["lcd_y"] + 8)
        if len(points_b) > 1:
            pygame.draw.lines(surface, Colors.CYAN, False, points_b, 1)
        
        # Status bar
        problems = self.app.simulator.get_all_problems()
        if problems:
            status = " | ".join(problems)
            status_color = Colors.RED
        else:
            status = "OK - " + T("active")
            status_color = Colors.GREEN
        
        status_surf = self.app.font_small.render(status, True, status_color)
        surface.blit(status_surf, (50, layout["status_y"]))


class PageDetails(BasePage):
    """Page DÉTAILS - Métriques détaillées avec Nixie bars"""
    
    def draw(self, surface: pygame.Surface):
        layout = DETAILS_LAYOUT
        data = self.app.simulator.data
        
        # Titre
        title = T("page_details")
        title_surf = self.app.font_large.render(title, True, Colors.AMBER)
        surface.blit(title_surf, (SCREEN_WIDTH // 2 - title_surf.get_width() // 2,
                                  layout["title_y"]))
        
        # Rail A
        self._draw_rail_details(surface, data.rail_a, T("rail_a"), 
                               layout["rail_a_x"], layout["metrics_y"], Colors.GREEN)
        
        # Rail B
        self._draw_rail_details(surface, data.rail_b, T("rail_b"),
                               layout["rail_b_x"], layout["metrics_y"], Colors.CYAN)
    
    def _draw_rail_details(self, surface: pygame.Surface, rail: RailData,
                          title: str, x: int, y: int, color: Tuple[int, int, int]):
        layout = DETAILS_LAYOUT
        
        # Titre rail
        title_surf = self.app.font_medium.render(title, True, color)
        surface.blit(title_surf, (x, y))
        y += 30
        
        metrics = [
            (T("voltage"), f"{rail.voltage_actual:.2f}", "V", rail.voltage_actual, 15.0),
            (T("current"), f"{rail.current_ma:.0f}", "mA", rail.current_ma, 500.0),
            (T("power"), f"{rail.power_w:.2f}", "W", rail.power_w, 5.0),
            (T("temperature"), f"{rail.temperature_c:.1f}", "°C", rail.temperature_c, 100.0),
            (T("ripple"), f"{rail.ripple_uv:.1f}", "µV", rail.ripple_uv, 50.0),
            (T("headroom"), f"{rail.headroom_v:.1f}", "V", rail.headroom_v, 5.0),
        ]
        
        for label, value, unit, current, max_val in metrics:
            # Label
            label_surf = self.app.font_small.render(f"{label}:", True, Colors.LIGHT_GRAY)
            surface.blit(label_surf, (x, y))
            
            # Valeur
            val_surf = self.app.font_small.render(f"{value} {unit}", True, Colors.WHITE)
            surface.blit(val_surf, (x + 120, y))
            
            # Barre Nixie
            NixieBar.draw(surface, x + 220, y + 2, 150, layout["bar_height"] - 4,
                         current, max_val, color)
            
            y += layout["spacing"]


class PageHealth(BasePage):
    """Page SANTÉ - Statut système et protections"""
    
    def draw(self, surface: pygame.Surface):
        layout = HEALTH_LAYOUT
        data = self.app.simulator.data
        
        # Titre
        title = T("page_health")
        title_surf = self.app.font_large.render(title, True, Colors.AMBER)
        surface.blit(title_surf, (SCREEN_WIDTH // 2 - title_surf.get_width() // 2,
                                  layout["title_y"]))
        
        # Statut global
        problems = self.app.simulator.get_all_problems()
        if problems:
            status = T("warning")
            status_color = Colors.RED
        else:
            status = T("ok")
            status_color = Colors.GREEN
        
        status_surf = self.app.font_large.render(f"SYSTÈME: {status}", True, status_color)
        surface.blit(status_surf, (50, layout["status_y"]))
        
        # Liste des problèmes
        y = layout["status_y"] + 40
        if problems:
            for prob in problems:
                prob_surf = self.app.font_small.render(f"⚠ {prob}", True, Colors.RED)
                surface.blit(prob_surf, (70, y))
                y += 25
        
        # Protections
        y = layout["protection_y"]
        prot_title = self.app.font_medium.render("PROTECTIONS:", True, Colors.AMBER)
        surface.blit(prot_title, (50, y))
        y += 30
        
        protections = [
            (T("ovp"), data.rail_a.ovp_active or data.rail_b.ovp_active),
            (T("ocp"), data.rail_a.ocp_active or data.rail_b.ocp_active),
            (T("otp"), data.rail_a.otp_active or data.rail_b.otp_active),
        ]
        
        for name, active in protections:
            color = Colors.RED if active else Colors.GREEN
            status_text = T("active") if active else T("ok")
            text = f"{name}: {status_text}"
            text_surf = self.app.font_small.render(text, True, color)
            surface.blit(text_surf, (70, y))
            y += 25
        
        # Températures
        y = layout["temp_y"]
        temp_title = self.app.font_medium.render(f"{T('temperature')}:", True, Colors.AMBER)
        surface.blit(temp_title, (50, y))
        y += 30
        
        temp_a = f"{T('rail_a')}: {data.rail_a.temperature_c:.1f}°C"
        temp_b = f"{T('rail_b')}: {data.rail_b.temperature_c:.1f}°C"
        
        color_a = Colors.RED if data.rail_a.temperature_c > 70 else Colors.GREEN
        color_b = Colors.RED if data.rail_b.temperature_c > 70 else Colors.GREEN
        
        surface.blit(self.app.font_small.render(temp_a, True, color_a), (70, y))
        surface.blit(self.app.font_small.render(temp_b, True, color_b), (70, y + 25))


class PageSession(BasePage):
    """Page SESSION - Timer et énergie"""
    
    def draw(self, surface: pygame.Surface):
        layout = SESSION_LAYOUT
        data = self.app.simulator.data
        
        # Titre
        title = T("page_session")
        title_surf = self.app.font_large.render(title, True, Colors.AMBER)
        surface.blit(title_surf, (SCREEN_WIDTH // 2 - title_surf.get_width() // 2,
                                  layout["title_y"]))
        
        # Timer
        uptime = timedelta(seconds=data.uptime_seconds)
        hours = uptime.seconds // 3600
        minutes = (uptime.seconds % 3600) // 60
        seconds = uptime.seconds % 60
        
        timer_text = f"{hours:02d}:{minutes:02d}:{seconds:02d}"
        timer_surf = self.app.font_xlarge.render(timer_text, True, Colors.GREEN)
        surface.blit(timer_surf, (SCREEN_WIDTH // 2 - timer_surf.get_width() // 2,
                                  layout["timer_y"]))
        
        uptime_label = T("uptime")
        label_surf = self.app.font_medium.render(uptime_label, True, Colors.LIGHT_GRAY)
        surface.blit(label_surf, (SCREEN_WIDTH // 2 - label_surf.get_width() // 2,
                                  layout["timer_y"] + 60))
        
        # Énergie
        energy_text = f"{data.energy_wh:.2f} Wh"
        energy_surf = self.app.font_large.render(energy_text, True, Colors.CYAN)
        surface.blit(energy_surf, (SCREEN_WIDTH // 2 - energy_surf.get_width() // 2,
                                   layout["energy_y"]))
        
        energy_label = T("energy")
        energy_label_surf = self.app.font_medium.render(energy_label, True, Colors.LIGHT_GRAY)
        surface.blit(energy_label_surf, (SCREEN_WIDTH // 2 - energy_label_surf.get_width() // 2,
                                         layout["energy_y"] + 40))
        
        # Stats
        y = layout["stats_y"]
        stats = [
            (T("session_start"), data.session_start.strftime("%H:%M:%S")),
            (f"{T('power')} A", f"{data.rail_a.power_w:.2f} W"),
            (f"{T('power')} B", f"{data.rail_b.power_w:.2f} W"),
        ]
        
        for label, value in stats:
            text = f"{label}: {value}"
            text_surf = self.app.font_small.render(text, True, Colors.LIGHT_GRAY)
            surface.blit(text_surf, (50, y))
            y += 25


class PageConfig(BasePage):
    """Page CONFIG - Paramètres et simulations"""
    
    def __init__(self, app: 'LPSDuoProApp'):
        super().__init__(app)
        self.sim_modes = [
            (SimulationMode.NORMAL, "sim_normal"),
            (SimulationMode.HOT, "sim_hot"),
            (SimulationMode.RIPPLE, "sim_ripple"),
            (SimulationMode.LOAD, "sim_load"),
            (SimulationMode.LOW_V, "sim_lo_v"),
            (SimulationMode.HIGH_V, "sim_hi_v"),
        ]
        self.buttons: List[Button] = []
        self._create_buttons()
    
    def _create_buttons(self):
        layout = CONFIG_LAYOUT
        x = 50
        for mode, key in self.sim_modes:
            btn = Button(x, layout["sim_buttons_y"], 100, 40, T(key),
                        lambda m=mode: self.app.simulator.set_simulation_mode(m))
            self.buttons.append(btn)
            x += 120
    
    def handle_event(self, event: pygame.event.Event):
        for btn in self.buttons:
            btn.handle_event(event)
    
    def draw(self, surface: pygame.Surface):
        layout = CONFIG_LAYOUT
        
        # Titre
        title = T("page_config")
        title_surf = self.app.font_large.render(title, True, Colors.AMBER)
        surface.blit(title_surf, (SCREEN_WIDTH // 2 - title_surf.get_width() // 2,
                                  layout["title_y"]))
        
        # Options
        y = layout["options_y"]
        options = [
            (T("language"), Translations.get_current_language().value),
            (T("brightness"), "80%"),
            (T("simulation"), self.app.simulator.data.simulation_mode.name),
        ]
        
        for label, value in options:
            text = f"{label}: {value}"
            text_surf = self.app.font_medium.render(text, True, Colors.WHITE)
            surface.blit(text_surf, (50, y))
            y += layout["option_height"]
        
        # Label simulation
        sim_label = self.app.font_medium.render(T("simulation") + ":", True, Colors.AMBER)
        surface.blit(sim_label, (50, layout["sim_buttons_y"] - 35))
        
        # Boutons simulation
        for btn in self.buttons:
            btn.draw(surface, self.app.font_small)


# =============================================================================
# APPLICATION PRINCIPALE
# =============================================================================

class LPSDuoProApp:
    """Application principale LPS DUO PRO"""
    
    def __init__(self):
        pygame.init()
        pygame.display.set_caption("LPS DUO PRO - Simulateur V92")
        
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        self.clock = pygame.time.Clock()
        self.running = True
        
        # Fonts
        self.font_small = pygame.font.Font(None, 20)
        self.font_medium = pygame.font.Font(None, 28)
        self.font_large = pygame.font.Font(None, 36)
        self.font_xlarge = pygame.font.Font(None, 72)
        
        # Simulateur de données
        self.simulator = DataSimulator()
        
        # Pages
        self.pages: List[BasePage] = [
            PageEcoute(self),
            PageDetails(self),
            PageHealth(self),
            PageSession(self),
            PageConfig(self),
        ]
        self.current_page = 0
        
        # État boot
        self.boot_screen = True
        self.boot_start = time.time()
    
    def handle_events(self):
        """Gestion des événements"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            
            elif event.type == pygame.KEYDOWN:
                if self.boot_screen:
                    if event.key == pygame.K_RETURN:
                        self.boot_screen = False
                else:
                    if event.key == pygame.K_ESCAPE:
                        self.running = False
                    elif event.key in (pygame.K_1, pygame.K_KP1):
                        self.current_page = 0
                    elif event.key in (pygame.K_2, pygame.K_KP2):
                        self.current_page = 1
                    elif event.key in (pygame.K_3, pygame.K_KP3):
                        self.current_page = 2
                    elif event.key in (pygame.K_4, pygame.K_KP4):
                        self.current_page = 3
                    elif event.key in (pygame.K_5, pygame.K_KP5):
                        self.current_page = 4
                    elif event.key == pygame.K_l:
                        Translations.next_language()
            
            # Passer les événements à la page courante
            if not self.boot_screen and self.current_page < len(self.pages):
                self.pages[self.current_page].handle_event(event)
    
    def update(self, dt: float):
        """Mise à jour logique"""
        if not self.boot_screen:
            self.simulator.update(dt)
            if self.current_page < len(self.pages):
                self.pages[self.current_page].update(dt)
    
    def draw_boot_screen(self):
        """Dessine l'écran de démarrage"""
        self.screen.fill(Colors.BLACK)
        
        # Logo / Titre
        title = "LPS DUO PRO"
        title_surf = self.font_xlarge.render(title, True, Colors.AMBER)
        self.screen.blit(title_surf, 
                        (SCREEN_WIDTH // 2 - title_surf.get_width() // 2, 150))
        
        # Sous-titre
        subtitle = "Alimentation Linéaire Audiophile"
        sub_surf = self.font_medium.render(subtitle, True, Colors.LIGHT_GRAY)
        self.screen.blit(sub_surf,
                        (SCREEN_WIDTH // 2 - sub_surf.get_width() // 2, 230))
        
        # Version
        version = "Simulateur PyGame V92"
        ver_surf = self.font_small.render(version, True, Colors.MID_GRAY)
        self.screen.blit(ver_surf,
                        (SCREEN_WIDTH // 2 - ver_surf.get_width() // 2, 280))
        
        # Animation de chargement
        elapsed = time.time() - self.boot_start
        dots = "." * (int(elapsed * 2) % 4)
        loading = f"Initialisation{dots}"
        load_surf = self.font_medium.render(loading, True, Colors.GREEN)
        self.screen.blit(load_surf,
                        (SCREEN_WIDTH // 2 - load_surf.get_width() // 2, 350))
        
        # Instruction
        instruction = "Appuyez sur ENTER pour démarrer"
        inst_surf = self.font_small.render(instruction, True, Colors.AMBER)
        self.screen.blit(inst_surf,
                        (SCREEN_WIDTH // 2 - inst_surf.get_width() // 2, 420))
    
    def draw_nav_bar(self):
        """Dessine la barre de navigation"""
        nav_y = SCREEN_HEIGHT - 35
        pygame.draw.rect(self.screen, Colors.DARK_GRAY,
                        (0, nav_y, SCREEN_WIDTH, 35))
        pygame.draw.line(self.screen, Colors.MID_GRAY,
                        (0, nav_y), (SCREEN_WIDTH, nav_y))
        
        pages = [
            T("page_listen"), T("page_details"), T("page_health"),
            T("page_session"), T("page_config")
        ]
        
        x = 20
        for i, name in enumerate(pages):
            color = Colors.AMBER if i == self.current_page else Colors.LIGHT_GRAY
            text = f"[{i+1}] {name}"
            text_surf = self.font_small.render(text, True, color)
            self.screen.blit(text_surf, (x, nav_y + 10))
            x += text_surf.get_width() + 20
    
    def draw(self):
        """Rendu graphique"""
        self.screen.fill(Colors.BLACK)
        
        if self.boot_screen:
            self.draw_boot_screen()
        else:
            # Page courante
            if self.current_page < len(self.pages):
                self.pages[self.current_page].draw(self.screen)
            
            # Barre de navigation
            self.draw_nav_bar()
        
        pygame.display.flip()
    
    def run(self):
        """Boucle principale"""
        last_time = time.time()
        
        while self.running:
            current_time = time.time()
            dt = current_time - last_time
            last_time = current_time
            
            self.handle_events()
            self.update(dt)
            self.draw()
            
            self.clock.tick(TARGET_FPS)
        
        pygame.quit()


# =============================================================================
# DEBUG LOGGER
# =============================================================================

class DebugLogger:
    """Système de logging pour debug avec export"""
    
    def __init__(self, max_entries: int = 1000):
        self.entries: List[Dict[str, Any]] = []
        self.max_entries = max_entries
        self.start_time = datetime.now()
    
    def log(self, level: str, message: str, data: Optional[Dict] = None):
        """Ajoute une entrée de log"""
        entry = {
            "timestamp": datetime.now().isoformat(),
            "elapsed": (datetime.now() - self.start_time).total_seconds(),
            "level": level,
            "message": message,
            "data": data or {}
        }
        self.entries.append(entry)
        
        if len(self.entries) > self.max_entries:
            self.entries.pop(0)
    
    def info(self, message: str, data: Optional[Dict] = None):
        self.log("INFO", message, data)
    
    def warn(self, message: str, data: Optional[Dict] = None):
        self.log("WARN", message, data)
    
    def error(self, message: str, data: Optional[Dict] = None):
        self.log("ERROR", message, data)
    
    def export(self, filepath: str):
        """Exporte les logs vers un fichier"""
        import json
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(self.entries, f, indent=2, ensure_ascii=False)


# =============================================================================
# SYSTÈME DE CACHE STATIQUE
# =============================================================================

class StaticCache:
    """Cache statique pour optimisation des surfaces"""
    
    _cache: Dict[str, pygame.Surface] = {}
    
    @classmethod
    def get(cls, key: str) -> Optional[pygame.Surface]:
        """Récupère une surface du cache"""
        return cls._cache.get(key)
    
    @classmethod
    def set(cls, key: str, surface: pygame.Surface):
        """Stocke une surface dans le cache"""
        cls._cache[key] = surface
    
    @classmethod
    def get_or_create(cls, key: str, creator: Callable[[], pygame.Surface]) -> pygame.Surface:
        """Récupère ou crée une surface"""
        if key not in cls._cache:
            cls._cache[key] = creator()
        return cls._cache[key]
    
    @classmethod
    def clear(cls):
        """Vide le cache"""
        cls._cache.clear()


# =============================================================================
# POINT D'ENTRÉE
# =============================================================================

def main():
    """Point d'entrée principal"""
    print("=" * 60)
    print("  LPS DUO PRO - Simulateur PyGame V92")
    print("  Alimentation Linéaire Audiophile")
    print("=" * 60)
    print()
    print("IMPORTANT: Ce simulateur est pour le développement PC.")
    print("L'interface production tourne sur ESP32-8048S050C avec LVGL.")
    print()
    print("Raccourcis:")
    print("  1-5    : Navigation entre les pages")
    print("  L      : Changer la langue")
    print("  ESC    : Quitter")
    print("  ENTER  : Démarrer (écran boot)")
    print()
    
    app = LPSDuoProApp()
    app.run()


if __name__ == "__main__":
    main()
