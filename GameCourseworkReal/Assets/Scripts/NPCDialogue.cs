using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Cinemachine;
using StarterAssets;

[RequireComponent(typeof(Collider))]
public class NPCDialogue : MonoBehaviour
{
    [System.Serializable]
    public class DialogueState
    {
 
        public InventoryItem requiredItemToUnlock;

        [TextArea(3, 6)]
        public string[] lines;
    }

    [Header("Conversation States")]
    public DialogueState[] states;

    [Header("UI (Assign in Inspector)")]
    public GameObject dialoguePanel;
    public Text dialogueText;
    public Text promptText;
    public AudioClip talkSfx;

    [Header("Cinemachine")]
    public CinemachineVirtualCamera vCam;

    bool _playerInZone = false;
    bool _inDialogue = false;
    int _currentState = 0;
    int _currentLine = 0;
    Transform _originalLookAt;
    StarterAssetsInputs _input;
    FirstPersonController _fpc;

    void Start()
    {
        if (dialoguePanel != null) dialoguePanel.SetActive(false);
        if (promptText != null) promptText.enabled = false;

        _fpc = Object.FindFirstObjectByType<FirstPersonController>();
        _input = Object.FindFirstObjectByType<StarterAssetsInputs>();

        if (vCam != null) _originalLookAt = vCam.LookAt;

        var inv = Object.FindFirstObjectByType<Inventory>();
        if (inv != null)
            inv.ItemUsed += OnItemUsed;
        else
            Debug.LogWarning($"[{name}] No Inventory found to subscribe.");
    }

    void OnDestroy()
    {
        var inv = Object.FindFirstObjectByType<Inventory>();
        if (inv != null)
            inv.ItemUsed -= OnItemUsed;
    }

    void Update()
    {
        if (_inDialogue)
        {
            if (Input.GetKeyDown(KeyCode.E))
                AdvanceLine();
            return;
        }

        if (_playerInZone)
        {
            if (promptText != null) promptText.enabled = true;
            if (Input.GetKeyDown(KeyCode.E))
                StartDialogue();
        }
        else if (promptText != null)
        {
            promptText.enabled = false;
        }
    }

    void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
            _playerInZone = true;
    }

    void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
            _playerInZone = false;
    }

    void OnItemUsed(InventoryItem item)
    {
        int next = _currentState + 1;
        if (states != null && next < states.Length && states[next].requiredItemToUnlock == item)
        {
            _currentState = next;
            Debug.Log($"[{name}] Unlocked dialogue state {next}");
        }
    }

    void StartDialogue()
    {

        if (states == null || states.Length == 0)
            return;

        _inDialogue = true;
        _currentLine = 0;

        if (dialoguePanel != null) dialoguePanel.SetActive(true);
        if (_fpc != null) _fpc.enabled = false;
        if (_input != null) _input.enabled = false;
        if (vCam != null) vCam.LookAt = transform;

        ShowLine();
        AudioSource.PlayClipAtPoint(talkSfx, Camera.main.transform.position, 0.8f);
    }

    void AdvanceLine()
    {
        _currentLine++;
        var lines = states[_currentState].lines;
        if (lines != null && _currentLine < lines.Length)
        {
            ShowLine();
        }
        else
        {
            EndDialogue();
        }
    }

    void ShowLine()
    {
        var lines = states[_currentState].lines;
        if (dialogueText != null && lines != null && _currentLine < lines.Length)
            dialogueText.text = lines[_currentLine];
    }

    void EndDialogue()
    {
        _inDialogue = false;
        if (dialoguePanel != null) dialoguePanel.SetActive(false);
        if (_fpc != null) _fpc.enabled = true;
        if (_input != null) _input.enabled = true;
        if (vCam != null && _originalLookAt != null)
            vCam.LookAt = _originalLookAt;
    }
}
