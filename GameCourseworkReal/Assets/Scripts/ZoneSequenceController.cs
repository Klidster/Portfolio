using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Cinemachine;
using StarterAssets;

[RequireComponent(typeof(Collider))]
public class ZoneSequenceController : MonoBehaviour
{
    [Header("Activation")]
    public Inventory inventory;
    public InventoryItem requiredItem;
    public AudioClip glueSfx;
    public AudioClip victorySfx;

    [Header("Camera")]
    public CinemachineVirtualCamera zoneCamera;
    int _originalCamPriority;

    [Header("Player Lock")]
    public FirstPersonController fpc;
    public StarterAssetsInputs inputs;

    [Header("Sequence Steps")]
    public Transform[] positions;

    [Header("UI Panels")]
    public GameObject dialoguePanel;
    public Text instructionText;
    public GameObject blackFade;
    public GameObject gameOverPanel;

    [Header("Canvas Root")]
    public GameObject canvasRoot;

    bool _inZone = false;
    bool _active = false;
    int _step = 0;

    void Awake()
    {

        var col = GetComponent<Collider>();
        col.isTrigger = true;

        if (dialoguePanel != null) dialoguePanel.SetActive(false);
        if (blackFade != null) blackFade.SetActive(false);
        if (gameOverPanel != null) gameOverPanel.SetActive(false);
    }

    void Start()
    {
        if (zoneCamera != null)
            _originalCamPriority = zoneCamera.Priority;

        if (fpc == null) fpc = FindObjectOfType<FirstPersonController>();
        if (inputs == null) inputs = FindObjectOfType<StarterAssetsInputs>();
    }

    void OnEnable()
    {
        inventory.ItemUsed += OnItemUsed;
    }

    void OnDisable()
    {
        inventory.ItemUsed -= OnItemUsed;
    }

    void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
            _inZone = true;
    }

    void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
            _inZone = false;
    }

    void OnItemUsed(InventoryItem item)
    {
        if (_active || !_inZone || item != requiredItem)
            return;

        StartSequence();
        AudioSource.PlayClipAtPoint(glueSfx, Camera.main.transform.position, 0.8f);
    }

    void StartSequence()
    {
        _active = true;
        _step = 0;

        if (fpc != null) fpc.enabled = false;
        if (inputs != null) inputs.enabled = false;

        if (zoneCamera != null)
            zoneCamera.Priority = _originalCamPriority + 10;

        HideMainHUD();

        if (blackFade != null) blackFade.SetActive(true);
        if (dialoguePanel != null) dialoguePanel.SetActive(true);

        UpdateInstruction();
        TeleportToStep();
    }

    void Update()
    {
        if (!_active) return;

        if (_step < positions.Length)
        {
            if (_step % 2 == 0 && Input.GetKeyDown(KeyCode.A))
                NextStep();
            else if (_step % 2 == 1 && Input.GetKeyDown(KeyCode.D))
                NextStep();
        }
        else
        {
            EndSequence();
        }
    }

    void NextStep()
    {
        _step++;
        if (_step < positions.Length)
        {
            TeleportToStep();
            UpdateInstruction();
        }
    }

    void TeleportToStep()
    {
        if (_step < positions.Length && fpc != null)
        {
            var t = positions[_step];
            var pt = fpc.transform;
            pt.position = t.position;
            pt.rotation = t.rotation;
        }
    }

    void UpdateInstruction()
    {
        if (instructionText == null) return;
        if (_step < positions.Length)
        {
            instructionText.text =
                (_step % 2 == 0)
                ? "Press 'a'"
                : "Press 'd'";
        }
        else
        {
            instructionText.text = "";
        }
    }

    void HideMainHUD()
    {
        if (canvasRoot == null) return;
        foreach (Transform child in canvasRoot.transform)
        {
            var go = child.gameObject;
            if (go == dialoguePanel ||
                go == blackFade ||
                go == gameOverPanel)
                continue;
            go.SetActive(false);
        }
    }

    void EndSequence()
    {
        _active = false;

        if (dialoguePanel != null) dialoguePanel.SetActive(false);
        if (blackFade != null) blackFade.SetActive(false);

        if (gameOverPanel != null)
        {
            gameOverPanel.SetActive(true);
            AudioSource.PlayClipAtPoint(victorySfx, Camera.main.transform.position, 0.7f);

            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }
    }
}
