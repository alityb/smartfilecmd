#!/usr/bin/env python3
"""
Tests for the Python frontend components.
"""

import pytest
import os
import tempfile
from unittest.mock import Mock, patch, MagicMock
from pathlib import Path

# Add parent directory to path for imports
import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "python_frontend"))

from gemini_parser import GeminiParser
import utils

class TestGeminiParser:
    """Test the Gemini API parser."""
    
    def test_init_with_api_key(self):
        """Test initialization with explicit API key."""
        parser = GeminiParser(api_key="test-key")
        assert parser.api_key == "test-key"
    
    def test_init_without_api_key(self):
        """Test initialization without API key raises error."""
        with patch.dict(os.environ, {}, clear=True):
            with pytest.raises(ValueError, match="GEMINI_API_KEY environment variable not set"):
                GeminiParser()
    
    def test_init_with_env_api_key(self):
        """Test initialization with environment variable."""
        with patch.dict(os.environ, {'GEMINI_API_KEY': 'env-key'}):
            parser = GeminiParser()
            assert parser.api_key == 'env-key'
    
    @patch('requests.post')
    def test_parse_command_success(self, mock_post):
        """Test successful command parsing."""
        # Mock successful API response
        mock_response = Mock()
        mock_response.json.return_value = {
            'candidates': [{
                'content': {
                    'parts': [{'text': '{"action": "delete", "file_ext": ".png", "src": "~/Downloads", "dst": "", "dry_run": false, "force": false}'}]
                }
            }]
        }
        mock_response.raise_for_status.return_value = None
        mock_post.return_value = mock_response
        
        parser = GeminiParser(api_key="test-key")
        result = parser.parse_command("delete all .pngs in Downloads")
        
        assert result['action'] == 'delete'
        assert result['file_ext'] == '.png'
        assert result['src'] == '~/Downloads'
        assert result['dst'] == ''
    
    def test_fallback_parse(self):
        """Test fallback parsing when API fails."""
        parser = GeminiParser(api_key="test-key")
        
        with patch.object(parser, '_call_gemini_api', side_effect=Exception("API Error")):
            result = parser.parse_command("delete all .pngs in Downloads")
            
            assert result['action'] == 'delete'
            assert result['file_ext'] == '.png'
            assert result['src'] == '~/Downloads'

class TestUtils:
    """Test utility functions."""
    
    def test_validate_command_valid(self):
        """Test validation of valid commands."""
        valid_commands = [
            {'action': 'delete', 'src': '~/Downloads', 'file_ext': '.png'},
            {'action': 'move', 'src': '~/Downloads', 'dst': '~/Pictures', 'file_ext': '.jpg'},
            {'action': 'copy', 'src': '~/Documents', 'dst': '~/Backup', 'file_ext': '.pdf'},
            {'action': 'create_folder', 'src': '', 'dst': '~/Projects'},
        ]
        
        for cmd in valid_commands:
            is_valid, error_msg = utils.validate_command(cmd)
            assert is_valid, f"Command {cmd} should be valid: {error_msg}"
    
    def test_validate_command_invalid(self):
        """Test validation of invalid commands."""
        invalid_commands = [
            ({}, "Missing required field: action"),
            ({'action': 'invalid'}, "Invalid action: invalid"),
            ({'action': 'delete'}, "Missing required field: src"),
            ({'action': 'move', 'src': '~/Downloads'}, "Destination path is required for this operation"),
        ]
        
        for cmd, expected_error in invalid_commands:
            is_valid, error_msg = utils.validate_command(cmd)
            assert not is_valid
            assert expected_error in error_msg
    
    def test_expand_path(self):
        """Test path expansion."""
        # Test home directory expansion
        with patch.dict(os.environ, {'HOME': '/home/test'}):
            assert utils.expand_path('~/Downloads') == '/home/test/Downloads'
        
        # Test absolute path (should remain unchanged)
        assert utils.expand_path('/absolute/path') == '/absolute/path'
    
    def test_format_dry_run(self):
        """Test dry-run formatting."""
        commands = [
            {
                'action': 'delete',
                'src': '~/Downloads',
                'file_ext': '.png',
                'dst': ''
            },
            {
                'action': 'move',
                'src': '~/Downloads',
                'dst': '~/Pictures',
                'file_ext': '.jpg'
            },
            {
                'action': 'create_folder',
                'src': '',
                'dst': '~/Projects'
            }
        ]
        
        expected_formats = [
            "Would delete all .png files in ~/Downloads",
            "Would move all .jpg files from ~/Downloads to ~/Pictures",
            "Would create folder: ~/Projects"
        ]
        
        for cmd, expected in zip(commands, expected_formats):
            result = utils.format_dry_run(cmd)
            assert result == expected
    
    def test_format_operation_result_success(self):
        """Test formatting of successful operation results."""
        result = {
            'success': True,
            'message': 'Successfully moved 5 files',
            'files_scanned': 100,
            'files_matched': 5,
            'files_affected': 5,
            'duration_ms': 150
        }
        
        formatted = utils.format_operation_result(result)
        
        assert '✅ Successfully moved 5 files' in formatted
        assert '📊 Scanned: 100 files in 150ms' in formatted
        assert '🎯 Matched: 5 files' in formatted
        assert '⚡ Operation: 5 files affected' in formatted
    
    def test_format_operation_result_error(self):
        """Test formatting of error operation results."""
        result = {
            'success': False,
            'message': 'Permission denied'
        }
        
        formatted = utils.format_operation_result(result)
        assert '❌ Error: Permission denied' in formatted
    
    @patch('builtins.input', return_value='yes')
    def test_confirm_destructive_operation_yes(self, mock_input):
        """Test confirmation for destructive operations."""
        cmd = {'action': 'delete', 'src': '~/Downloads', 'file_ext': '.png'}
        
        with patch.object(utils, 'expand_path', return_value='/home/test/Downloads'):
            result = utils.confirm_destructive_operation(cmd)
            assert result is True
    
    @patch('builtins.input', return_value='no')
    def test_confirm_destructive_operation_no(self, mock_input):
        """Test confirmation rejection for destructive operations."""
        cmd = {'action': 'delete', 'src': '~/Downloads', 'file_ext': '.png'}
        
        with patch.object(utils, 'expand_path', return_value='/home/test/Downloads'):
            result = utils.confirm_destructive_operation(cmd)
            assert result is False
    
    def test_confirm_destructive_operation_non_destructive(self):
        """Test confirmation for non-destructive operations."""
        cmd = {'action': 'copy', 'src': '~/Downloads', 'dst': '~/Backup'}
        
        result = utils.confirm_destructive_operation(cmd)
        assert result is True  # Should auto-confirm non-destructive ops

if __name__ == '__main__':
    pytest.main([__file__])
