using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class InventoryHud : MonoBehaviour
{

    public Inventory inventory;
    private Transform panel;

    void Start()
    {
        panel = transform.Find("InventoryHud");
        if (panel == null)
            Debug.LogError("InventoryHud: Couldn't find child named 'InventoryHud'");

        inventory.ItemAdded += InventoryItemAdded;
        inventory.ItemRemoved += InventoryItemRemoved;
    }

    private void InventoryItemAdded(InventoryItem item)
    {
        Transform panel = transform.Find("InventoryHud");

        foreach (Transform slot in panel)
        {
            Image image = slot.GetComponent<Image>();
            InventoryItemClickable button = slot.GetComponent<InventoryItemClickable>();
            if (!image.enabled)
            {
                image.enabled = true;
                image.sprite = item.itemImage;
                button.item = item;
                break;
            }
        }
    }


    private void InventoryItemRemoved(InventoryItem item)
    {
        Transform panel = transform.Find("InventoryHud");

        foreach (Transform slot in panel)
        {
            Image image = slot.GetComponent<Image>();
            if (image.enabled && image.sprite == item.itemImage)
            {
                image.enabled = false;
                break;
            }
        }
    }

    void Update()
    {
        if (panel == null) return;

        if (Input.GetKeyDown(KeyCode.Alpha1)) UseSlot(0);
        if (Input.GetKeyDown(KeyCode.Alpha2)) UseSlot(1);
        if (Input.GetKeyDown(KeyCode.Alpha3)) UseSlot(2);
        if (Input.GetKeyDown(KeyCode.Alpha4)) UseSlot(3);
        if (Input.GetKeyDown(KeyCode.Alpha5)) UseSlot(4);
    }

    private void UseSlot(int index)
    {
        if (index < 0 || index >= panel.childCount) return;

        var slot = panel.GetChild(index);
        var clickHandler = slot.GetComponent<InventoryItemClickable>();
        if (clickHandler != null && clickHandler.item != null)
            inventory.useItem(clickHandler.item);
    }
}
