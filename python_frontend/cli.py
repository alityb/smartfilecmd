#!/usr/bin/env python3
"""
SmartFileCmd - Natural Language File Manager CLI
Main command-line interface for the application.
"""

import sys
import typer
from pathlib import Path
from gemini_parser import GeminiParser
from utils import validate_command, send_command_to_backend, format_result

app = typer.Typer(
    name="smartfilecli",
    help="SmartFileCmd - Natural Language File Manager",
    add_completion=False
)

@app.command()
def main(
    command: str = typer.Argument(..., help="Natural language command (e.g., 'remove all .txt files')"),
    dry_run: bool = typer.Option(False, "--dry-run", "-d", help="Preview mode - show what would be done"),
    force: bool = typer.Option(False, "--force", "-f", help="Skip confirmation prompts"),
    recursive: bool = typer.Option(False, "--recursive", "-r", help="Scan subdirectories recursively"),
    verbose: bool = typer.Option(False, "--verbose", "-v", help="Show detailed output")
):
    """
    SmartFileCmd - Natural Language File Manager
    
    Examples:
        smartfilecli "delete all .png files in Downloads"
        smartfilecli "move all .jpegs in Downloads to Pictures" --dry-run
        smartfilecli "copy all PDFs from Documents to Backup" --recursive
        smartfilecli "create a new folder called Projects in Documents"
    """
    
    # Initialize Gemini parser
    parser = GeminiParser()  # Will use GEMINI_API_KEY from environment
    
    try:
        # Parse natural language command
        if verbose:
            typer.echo(f"🔍 Parsing command: {command}")
        
        parsed_command = parser.parse_command(command)
        
        if not parsed_command:
            typer.echo("❌ Failed to parse command. Please try rephrasing.")
            sys.exit(1)
        
        if verbose:
            typer.echo(f"✅ Parsed command: {parsed_command}")
        
        # Validate command
        if not validate_command(parsed_command):
            typer.echo("❌ Invalid command structure")
            sys.exit(1)
        
        # Add CLI flags to the parsed command
        parsed_command.update({
            'dry_run': dry_run,
            'force': force,
            'recursive': recursive,
            'verbose': verbose
        })
        
        # Execute command
        if verbose:
            typer.echo("🚀 Executing command...")
        
        result = send_command_to_backend(parsed_command)
        
        if result:
            # Format and display result
            formatted_result = format_result(result, verbose=verbose)
            typer.echo(formatted_result)
            
            if not result.get("success", False):
                sys.exit(1)
        else:
            typer.echo("❌ Failed to communicate with backend")
            sys.exit(1)
            
    except KeyboardInterrupt:
        typer.echo("\n⚠️ Operation cancelled by user")
        sys.exit(1)
    except Exception as e:
        if verbose:
            typer.echo(f"❌ Unexpected error: {e}")
            import traceback
            traceback.print_exc()
        else:
            typer.echo(f"❌ Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    app()
