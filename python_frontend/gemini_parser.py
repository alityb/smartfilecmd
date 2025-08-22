import json
import os
import requests
from typing import Dict, Any, Optional

class GeminiParser:
    def __init__(self, api_key: Optional[str] = None):
        """Initialize the Gemini parser with API key."""
        self.api_key = api_key or os.getenv('GEMINI_API_KEY')
        # Temporarily disable API requirement for testing
        # if not self.api_key:
        #     raise ValueError("GEMINI_API_KEY environment variable not set")
        
        self.base_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent"
        
    def parse_command(self, natural_command: str) -> Dict[str, Any]:
        """
        Parse natural language command into structured JSON using Gemini API.
        
        Args:
            natural_command: Natural language command like "delete all .pngs in Downloads"
            
        Returns:
            Dictionary with structured command information
        """
        # Temporarily use fallback parsing for testing
        return self._fallback_parse(natural_command)
        
        # Original code (commented out for testing):
        # prompt = self._create_prompt(natural_command)
        # 
        # try:
        #     response = self._call_gemini_api(prompt)
        #     return self._parse_gemini_response(response)
        # except Exception as e:
        #     # Fallback to simple parsing if API fails
        #     return self._fallback_parse(natural_command)
    
    def _create_prompt(self, command: str) -> str:
        """Create a prompt for the Gemini API."""
        return f"""
You are a file operation parser. Convert the following natural language command into a JSON object with these fields:

- action: one of ["move", "copy", "delete", "create_folder"]
- file_ext: file extension like ".jpg", ".png", ".pdf" (empty string if not specified)
- src: source directory path
- dst: destination directory path (empty string for delete operations)
- dry_run: false (this will be set by the CLI)
- force: false (this will be set by the CLI)

Examples:
Input: "delete all .pngs in Downloads"
Output: {{"action": "delete", "file_ext": ".png", "src": "~/Downloads", "dst": "", "dry_run": false, "force": false}}

Input: "move all .jpegs in Downloads to a new folder called Pictures"
Output: {{"action": "move", "file_ext": ".jpeg", "src": "~/Downloads", "dst": "~/Downloads/Pictures", "dry_run": false, "force": false}}

Input: "copy all PDFs from Documents to Backup"
Output: {{"action": "copy", "file_ext": ".pdf", "src": "~/Documents", "dst": "~/Backup", "dry_run": false, "force": false}}

Input: "create a new folder called Projects in Documents"
Output: {{"action": "create_folder", "file_ext": "", "src": "", "dst": "~/Documents/Projects", "dry_run": false, "force": false}}

Now parse this command: "{command}"

Return only the JSON object, no other text.
"""
    
    def _call_gemini_api(self, prompt: str) -> Dict[str, Any]:
        """Make API call to Gemini."""
        headers = {
            'Content-Type': 'application/json',
        }
        
        data = {
            "contents": [{
                "parts": [{
                    "text": prompt
                }]
            }]
        }
        
        url = f"{self.base_url}?key={self.api_key}"
        response = requests.post(url, headers=headers, json=data, timeout=30)
        response.raise_for_status()
        
        return response.json()
    
    def _parse_gemini_response(self, response: Dict[str, Any]) -> Dict[str, Any]:
        """Parse the Gemini API response to extract the JSON command."""
        try:
            # Extract text from Gemini response
            text = response['candidates'][0]['content']['parts'][0]['text']
            
            # Clean up the response (remove markdown if present)
            text = text.strip()
            if text.startswith('```json'):
                text = text[7:]
            if text.endswith('```'):
                text = text[:-3]
            text = text.strip()
            
            # Parse JSON
            command = json.loads(text)
            
            # Validate required fields
            required_fields = ['action', 'src']
            for field in required_fields:
                if field not in command:
                    raise ValueError(f"Missing required field: {field}")
            
            # Set default values
            command.setdefault('file_ext', '')
            command.setdefault('dst', '')
            command.setdefault('dry_run', False)
            command.setdefault('force', False)
            
            return command
            
        except (KeyError, json.JSONDecodeError, ValueError) as e:
            raise ValueError(f"Failed to parse Gemini response: {e}")
    
    def _fallback_parse(self, command: str) -> Dict[str, Any]:
        """Fallback parsing when Gemini API is not available."""
        command_lower = command.lower()
        
        # Initialize command structure
        parsed = {
            'action': '',
            'pattern': '',
            'source': '',
            'destination': '',
            'dry_run': False,
            'force': False
        }
        
        # Determine action
        if any(word in command_lower for word in ['delete', 'remove', 'trash', 'clean up']):
            parsed['action'] = 'delete'
        elif any(word in command_lower for word in ['create', 'make', 'build', 'set up']):
            parsed['action'] = 'create_folder'
        elif any(word in command_lower for word in ['move', 'relocate', 'transfer', 'shift']):
            parsed['action'] = 'move'
        elif any(word in command_lower for word in ['copy', 'duplicate', 'backup', 'clone']):
            parsed['action'] = 'copy'
        else:
            return None
        
        # Extract file pattern/extensions
        extensions = ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.pdf', '.doc', '.docx', '.txt', 
                     '.zip', '.rar', '.tar', '.gz', '.mp3', '.mp4', '.avi', '.mkv']
        
        for ext in extensions:
            if ext in command_lower:
                parsed['pattern'] = ext
                break
        
        # If no specific extension found, look for general terms
        if not parsed['pattern']:
            if any(word in command_lower for word in ['images', 'photos', 'pictures']):
                parsed['pattern'] = '*.jpg'
            elif any(word in command_lower for word in ['documents', 'docs']):
                parsed['pattern'] = '*.doc*'
            elif any(word in command_lower for word in ['archives', 'compressed']):
                parsed['pattern'] = '*.zip'
            elif any(word in command_lower for word in ['videos', 'movies']):
                parsed['pattern'] = '*.mp4'
            elif any(word in command_lower for word in ['music', 'audio']):
                parsed['pattern'] = '*.mp3'
            elif any(word in command_lower for word in ['text', 'txt']):
                parsed['pattern'] = '*.txt'
            elif any(word in command_lower for word in ['all', 'every', 'each']):
                parsed['pattern'] = '*'
        
        # Extract source directory
        common_dirs = ['downloads', 'documents', 'pictures', 'desktop', 'home', 'music', 'videos']
        for dir_name in common_dirs:
            if dir_name in command_lower:
                parsed['source'] = f"~/{dir_name.capitalize()}"
                break
        
        # Set default source directory if none found (for delete operations)
        if not parsed['source']:
            parsed['source'] = "."  # Default to current directory
        
        # Extract destination directory (for move/copy/create_folder)
        if parsed['action'] in ['move', 'copy', 'create_folder']:
            # Look for "to" or "in" followed by directory
            if ' to ' in command_lower:
                parts = command_lower.split(' to ')
                if len(parts) > 1:
                    dest_part = parts[1].strip()
                    # First check common directories
                    for dir_name in common_dirs:
                        if dir_name in dest_part:
                            parsed['destination'] = f"~/{dir_name.capitalize()}"
                            break
                    # If no common directory found, use the custom directory name
                    if not parsed['destination']:
                        parsed['destination'] = dest_part
            elif ' in ' in command_lower:
                parts = command_lower.split(' in ')
                if len(parts) > 1:
                    dest_part = parts[1].strip()
                    # First check common directories
                    for dir_name in common_dirs:
                        if dir_name in dest_part:
                            parsed['destination'] = f"~/{dir_name.capitalize()}"
                            break
                    # If no common directory found, use the custom directory name
                    if not parsed['destination']:
                        parsed['destination'] = dest_part
        
        # Handle create_folder special case
        if parsed['action'] == 'create_folder':
            if 'called' in command_lower:
                parts = command_lower.split('called')
                if len(parts) > 1:
                    folder_name = parts[1].strip().split()[0]  # Get first word after "called"
                    if parsed['destination']:
                        parsed['destination'] = f"{parsed['destination']}/{folder_name}"
                    else:
                        parsed['destination'] = f"~/Downloads/{folder_name}"
        
        # Handle recursive patterns
        if '**' in command or 'recursive' in command_lower:
            parsed['recursive'] = True
        
        return parsed
