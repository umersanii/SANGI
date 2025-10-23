#!/bin/bash

# AWS IoT Policy Update Script for SANGI Robot
# This script updates the AWS IoT policy to allow SANGI topics

echo "=== SANGI AWS IoT Policy Update ==="
echo ""
echo "This script will update your AWS IoT policy to allow:"
echo "  - sangi/* topics (for robot control and status)"
echo "  - workspace/* topics (for PC/Pi activity monitoring)"
echo ""

# Variables
POLICY_NAME="SANGI-Policy"  # Change this if your policy has a different name
POLICY_FILE="./certs/SANGI-Policy-Updated.json"
REGION="us-east-1"

echo "Policy Name: $POLICY_NAME"
echo "Policy File: $POLICY_FILE"
echo "Region: $REGION"
echo ""

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null; then
    echo "❌ ERROR: AWS CLI is not installed!"
    echo ""
    echo "To install AWS CLI:"
    echo "  Ubuntu/Debian: sudo apt install awscli"
    echo "  macOS: brew install awscli"
    echo "  Or visit: https://aws.amazon.com/cli/"
    echo ""
    exit 1
fi

# Check if policy file exists
if [ ! -f "$POLICY_FILE" ]; then
    echo "❌ ERROR: Policy file not found: $POLICY_FILE"
    exit 1
fi

echo "✓ AWS CLI found"
echo "✓ Policy file found"
echo ""

# Confirm before proceeding
read -p "Do you want to update the policy? (y/n): " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Update cancelled."
    exit 0
fi

echo ""
echo "Updating policy..."

# Create a new policy version
aws iot create-policy-version \
    --policy-name "$POLICY_NAME" \
    --policy-document file://"$POLICY_FILE" \
    --set-as-default \
    --region "$REGION"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Policy updated successfully!"
    echo ""
    echo "The SANGI robot can now:"
    echo "  ✓ Publish to sangi/* topics"
    echo "  ✓ Subscribe to sangi/* topics"
    echo "  ✓ Receive workspace/* activity data"
    echo ""
else
    echo ""
    echo "❌ Failed to update policy"
    echo ""
    echo "Manual Steps:"
    echo "1. Go to AWS IoT Console > Secure > Policies"
    echo "2. Find your policy (probably named 'SANGI-Policy' or similar)"
    echo "3. Click 'Edit active version'"
    echo "4. Replace the policy with the contents of:"
    echo "   $POLICY_FILE"
    echo "5. Click 'Save as new version'"
    echo ""
fi
